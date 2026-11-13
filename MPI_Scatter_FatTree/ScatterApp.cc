#include <omnetpp.h>

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

using namespace omnetpp;

namespace {

enum MessageKinds {
    START = 1,
    DATA = 2
};

struct ScatterState {
    int numNodes = 0;
    int completedCount = 0;
    long messagesSent = 0;
    long long appBytesSent = 0;
    bool finished = false;
    simtime_t startTime = SIMTIME_ZERO;
    std::vector<bool> completed;
};

ScatterState state;

std::vector<int> parseTargets(const char *text)
{
    std::vector<int> targets;
    std::stringstream stream(text ? text : "");
    std::string item;
    while (std::getline(stream, item, ',')) {
        if (!item.empty())
            targets.push_back(std::stoi(item));
    }
    return targets;
}

std::string formatTargets(const std::vector<int>& targets)
{
    std::stringstream stream;
    for (size_t i = 0; i < targets.size(); i++) {
        if (i != 0)
            stream << ",";
        stream << targets[i];
    }
    return stream.str();
}

} // namespace

class ScatterNode : public cSimpleModule
{
  private:
    int nodeId = -1;
    int numNodes = 0;
    int rootId = 0;
    int payloadBytes = 0;
    double linkBitrate = 1e9;
    std::string algorithm;
    simtime_t nextSendTime = SIMTIME_ZERO;

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;

  private:
    void resetRunState();
    void startScatter();
    void handleData(cPacket *packet);
    void sendNaiveScatter();
    void sendMpiScatter(std::vector<int> targets);
    void sendData(int destination, const std::vector<int>& targets);
    void markCompleted();
};

Define_Module(ScatterNode);

void ScatterNode::initialize()
{
    nodeId = par("nodeId").intValue();
    numNodes = par("numNodes").intValue();
    rootId = par("rootId").intValue();
    payloadBytes = par("payloadBytes").intValue();
    linkBitrate = par("linkBitrate").doubleValue();
    algorithm = par("algorithm").stringValue();
    nextSendTime = SIMTIME_ZERO;

    if (nodeId == rootId) {
        resetRunState();
        scheduleAt(par("startTime"), new cMessage("start", START));
    }
}

void ScatterNode::resetRunState()
{
    state.numNodes = numNodes;
    state.completedCount = 0;
    state.messagesSent = 0;
    state.appBytesSent = 0;
    state.finished = false;
    state.startTime = SIMTIME_ZERO;
    state.completed.assign(numNodes, false);
}

void ScatterNode::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        delete msg;
        startScatter();
        return;
    }

    if (msg->getKind() == DATA) {
        handleData(check_and_cast<cPacket *>(msg));
        return;
    }

    delete msg;
}

void ScatterNode::startScatter()
{
    state.startTime = simTime();
    markCompleted();

    if (algorithm == "naive")
        sendNaiveScatter();
    else {
        std::vector<int> targets;
        for (int destination = 0; destination < numNodes; destination++) {
            if (destination != rootId)
                targets.push_back(destination);
        }
        sendMpiScatter(targets);
    }
}

void ScatterNode::handleData(cPacket *packet)
{
    std::vector<int> targets = parseTargets(packet->par("targets").stringValue());
    delete packet;

    if (std::find(targets.begin(), targets.end(), nodeId) != targets.end())
        markCompleted();

    targets.erase(std::remove(targets.begin(), targets.end(), nodeId), targets.end());

    if (algorithm == "mpi_scatter")
        sendMpiScatter(targets);
}

void ScatterNode::sendNaiveScatter()
{
    for (int destination = 0; destination < numNodes; destination++) {
        if (destination != rootId)
            sendData(destination, {destination});
    }
}

void ScatterNode::sendMpiScatter(std::vector<int> targets)
{
    if (targets.empty())
        return;

    std::sort(targets.begin(), targets.end());

    if (targets.size() == 1) {
        sendData(targets.front(), targets);
        return;
    }

    size_t split = targets.size() / 2;
    std::vector<int> localTargets(targets.begin(), targets.begin() + split);
    std::vector<int> remoteTargets(targets.begin() + split, targets.end());

    sendData(remoteTargets.front(), remoteTargets);
    sendMpiScatter(localTargets);
}

void ScatterNode::sendData(int destination, const std::vector<int>& targets)
{
    auto *packet = new cPacket("scatterData", DATA);
    int bytes = payloadBytes * static_cast<int>(targets.size());
    packet->setByteLength(bytes);
    packet->addPar("dest") = destination;
    packet->addPar("source") = nodeId;
    packet->addPar("targets") = formatTargets(targets).c_str();

    state.messagesSent++;
    state.appBytesSent += bytes;

    if (nextSendTime < simTime())
        nextSendTime = simTime();

    simtime_t sendTime = nextSendTime;
    simtime_t serializationTime = SimTime(bytes * 8.0 / linkBitrate);
    nextSendTime += serializationTime;

    sendDelayed(packet, sendTime - simTime(), "out", 0);
}

void ScatterNode::markCompleted()
{
    if (nodeId < 0 || nodeId >= state.numNodes || state.completed[nodeId])
        return;

    state.completed[nodeId] = true;
    state.completedCount++;

    if (!state.finished && state.completedCount == state.numNodes) {
        state.finished = true;
        recordScalar("scatterCompletionTime", (simTime() - state.startTime).dbl());
        recordScalar("scatterMessagesSent", state.messagesSent);
        recordScalar("scatterAppBytesSent", state.appBytesSent);
        endSimulation();
    }
}

class FatTreeSwitch : public cSimpleModule
{
  private:
    std::string role;
    int minNodeId = -1;
    int maxNodeId = -1;
    double linkBitrate = 1e9;
    std::vector<simtime_t> nextOutputSendTimes;

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;

  private:
    int findOutputGateForDestination(int destination, int arrivalIndex) const;
    cModule *getOutputNeighbor(int gateIndex) const;
    bool neighborHasRole(int gateIndex, const char *expectedRole) const;
    int findNeighborInRange(int destination, const char *expectedRole, int arrivalIndex) const;
    int findNeighborByRole(const char *expectedRole, int arrivalIndex) const;
};

Define_Module(FatTreeSwitch);

void FatTreeSwitch::initialize()
{
    role = par("role").stringValue();
    minNodeId = par("minNodeId").intValue();
    maxNodeId = par("maxNodeId").intValue();
    linkBitrate = par("linkBitrate").doubleValue();
    nextOutputSendTimes.assign(gateSize("out"), SIMTIME_ZERO);
}

void FatTreeSwitch::handleMessage(cMessage *msg)
{
    auto *packet = check_and_cast<cPacket *>(msg);
    int destination = packet->par("dest").longValue();
    int arrivalIndex = packet->getArrivalGate()->getIndex();
    int outputIndex = findOutputGateForDestination(destination, arrivalIndex);

    if (outputIndex < 0)
        throw cRuntimeError("No route from %s to host %d", getFullPath().c_str(), destination);

    if (nextOutputSendTimes[outputIndex] < simTime())
        nextOutputSendTimes[outputIndex] = simTime();

    simtime_t sendTime = nextOutputSendTimes[outputIndex];
    simtime_t serializationTime = SimTime(packet->getByteLength() * 8.0 / linkBitrate);
    nextOutputSendTimes[outputIndex] += serializationTime;

    sendDelayed(packet, sendTime - simTime(), "out", outputIndex);
}

int FatTreeSwitch::findOutputGateForDestination(int destination, int arrivalIndex) const
{
    int outputSize = gateSize("out");

    for (int i = 0; i < outputSize; i++) {
        cModule *neighbor = getOutputNeighbor(i);
        if (neighbor != nullptr && neighbor->hasPar("nodeId") && neighbor->par("nodeId").intValue() == destination)
            return i;
    }

    if (role == "edge") {
        int aggregationGate = findNeighborByRole("aggregation", arrivalIndex);
        if (aggregationGate >= 0)
            return aggregationGate;
        return findNeighborByRole("core", arrivalIndex);
    }

    if (role == "aggregation") {
        int edgeGate = findNeighborInRange(destination, "edge", arrivalIndex);
        if (edgeGate >= 0)
            return edgeGate;
        return findNeighborByRole("core", arrivalIndex);
    }

    if (role == "core") {
        int aggregationGate = findNeighborInRange(destination, "aggregation", arrivalIndex);
        if (aggregationGate >= 0)
            return aggregationGate;
        return findNeighborInRange(destination, "edge", arrivalIndex);
    }

    for (int i = 0; i < outputSize; i++) {
        if (i != arrivalIndex)
            return i;
    }

    return -1;
}

cModule *FatTreeSwitch::getOutputNeighbor(int gateIndex) const
{
    const cGate *outputGate = gate("out", gateIndex);
    cGate *pathEndGate = outputGate->getPathEndGate();
    return pathEndGate == nullptr ? nullptr : pathEndGate->getOwnerModule();
}

bool FatTreeSwitch::neighborHasRole(int gateIndex, const char *expectedRole) const
{
    cModule *neighbor = getOutputNeighbor(gateIndex);
    return neighbor != nullptr && neighbor->hasPar("role") && std::string(neighbor->par("role").stringValue()) == expectedRole;
}

int FatTreeSwitch::findNeighborInRange(int destination, const char *expectedRole, int arrivalIndex) const
{
    int outputSize = gateSize("out");
    for (int i = 0; i < outputSize; i++) {
        if (i == arrivalIndex || !neighborHasRole(i, expectedRole))
            continue;

        cModule *neighbor = getOutputNeighbor(i);
        int minNode = neighbor->par("minNodeId").intValue();
        int maxNode = neighbor->par("maxNodeId").intValue();
        if (minNode <= destination && destination <= maxNode)
            return i;
    }

    return -1;
}

int FatTreeSwitch::findNeighborByRole(const char *expectedRole, int arrivalIndex) const
{
    int outputSize = gateSize("out");
    for (int i = 0; i < outputSize; i++) {
        if (i != arrivalIndex && neighborHasRole(i, expectedRole))
            return i;
    }

    return -1;
}
