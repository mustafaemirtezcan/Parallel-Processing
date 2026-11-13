simple ScatterNode
{
    parameters:
        int nodeId;
        int numNodes;
        int rootId = default(0);
        string algorithm = default("mpi_scatter");
        int payloadBytes = default(1048576);
        double linkBitrate @unit(bps) = default(1Gbps);
        double startTime @unit(s) = default(1s);
        @class(ScatterNode);
        @display("i=device/pc");
    gates:
        input in[];
        output out[];
}

simple FatTreeSwitch
{
    parameters:
        string role;
        int minNodeId = default(-1);
        int maxNodeId = default(-1);
        double linkBitrate @unit(bps) = default(1Gbps);
        @class(FatTreeSwitch);
        @display("i=device/switch");
    gates:
        input in[];
        output out[];
}

channel FatTreeLink extends ned.DatarateChannel
{
    delay = default(1us);
    datarate = default(1Gbps);
}

network ScatterFatTree4
{
    parameters:
        int payloadBytes = default(1048576);
        string algorithm = default("mpi_scatter");
        double linkBitrate @unit(bps) = default(1Gbps);
        @display("bgb=620,420");

    submodules:
        core: FatTreeSwitch {
            parameters:
                role = "core";
                linkBitrate = parent.linkBitrate;
                @display("p=310,90");
        }
        edge1: FatTreeSwitch {
            parameters:
                role = "edge";
                minNodeId = 0;
                maxNodeId = 1;
                linkBitrate = parent.linkBitrate;
                @display("p=170,210");
        }
        edge2: FatTreeSwitch {
            parameters:
                role = "edge";
                minNodeId = 2;
                maxNodeId = 3;
                linkBitrate = parent.linkBitrate;
                @display("p=450,210");
        }
        host[4]: ScatterNode {
            parameters:
                nodeId = index;
                numNodes = 4;
                rootId = 0;
                algorithm = parent.algorithm;
                payloadBytes = parent.payloadBytes;
                linkBitrate = parent.linkBitrate;
                @display("p=80,330,row,150");
        }

    connections:
        host[0].out++ --> FatTreeLink { datarate = parent.linkBitrate; } --> edge1.in++;
        edge1.out++ --> FatTreeLink { datarate = parent.linkBitrate; } --> host[0].in++;
        host[1].out++ --> FatTreeLink { datarate = parent.linkBitrate; } --> edge1.in++;
        edge1.out++ --> FatTreeLink { datarate = parent.linkBitrate; } --> host[1].in++;
        edge1.out++ --> FatTreeLink { datarate = parent.linkBitrate; } --> core.in++;
        core.out++ --> FatTreeLink { datarate = parent.linkBitrate; } --> edge1.in++;

        host[2].out++ --> FatTreeLink { datarate = parent.linkBitrate; } --> edge2.in++;
        edge2.out++ --> FatTreeLink { datarate = parent.linkBitrate; } --> host[2].in++;
        host[3].out++ --> FatTreeLink { datarate = parent.linkBitrate; } --> edge2.in++;
        edge2.out++ --> FatTreeLink { datarate = parent.linkBitrate; } --> host[3].in++;
        edge2.out++ --> FatTreeLink { datarate = parent.linkBitrate; } --> core.in++;
        core.out++ --> FatTreeLink { datarate = parent.linkBitrate; } --> edge2.in++;
}

network ScatterFatTree8
{
    parameters:
        int payloadBytes = default(1048576);
        string algorithm = default("mpi_scatter");
        double linkBitrate @unit(bps) = default(1Gbps);
        @display("bgb=900,520");

    submodules:
        core[2]: FatTreeSwitch {
            parameters:
                role = "core";
                linkBitrate = parent.linkBitrate;
                @display("p=330,80,row,240");
        }
        agg[4]: FatTreeSwitch {
            parameters:
                role = "aggregation";
                minNodeId = int(index / 2) * 4;
                maxNodeId = int(index / 2) * 4 + 3;
                linkBitrate = parent.linkBitrate;
                @display("p=170,190,row,180");
        }
        edge[4]: FatTreeSwitch {
            parameters:
                role = "edge";
                minNodeId = index * 2;
                maxNodeId = index * 2 + 1;
                linkBitrate = parent.linkBitrate;
                @display("p=170,310,row,180");
        }
        host[8]: ScatterNode {
            parameters:
                nodeId = index;
                numNodes = 8;
                rootId = 0;
                algorithm = parent.algorithm;
                payloadBytes = parent.payloadBytes;
                linkBitrate = parent.linkBitrate;
                @display("p=80,440,row,100");
        }

    connections:
        for i=0..7 {
            host[i].out++ --> FatTreeLink { datarate = parent.linkBitrate; } --> edge[int(i / 2)].in++;
            edge[int(i / 2)].out++ --> FatTreeLink { datarate = parent.linkBitrate; } --> host[i].in++;
        }

        for k=0..7 {
            edge[int(k / 2)].out++ --> FatTreeLink { datarate = parent.linkBitrate; } --> agg[int(int(k / 4) * 2 + k % 2)].in++;
            agg[int(int(k / 4) * 2 + k % 2)].out++ --> FatTreeLink { datarate = parent.linkBitrate; } --> edge[int(k / 2)].in++;
        }

        for k=0..7 {
            agg[int(k / 2)].out++ --> FatTreeLink { datarate = parent.linkBitrate; } --> core[k % 2].in++;
            core[k % 2].out++ --> FatTreeLink { datarate = parent.linkBitrate; } --> agg[int(k / 2)].in++;
        }
}

network ScatterFatTree16
{
    parameters:
        int payloadBytes = default(1048576);
        string algorithm = default("mpi_scatter");
        double linkBitrate @unit(bps) = default(1Gbps);
        @display("bgb=1120,620");

    submodules:
        core[4]: FatTreeSwitch {
            parameters:
                role = "core";
                linkBitrate = parent.linkBitrate;
                @display("p=300,80,row,170");
        }
        agg[8]: FatTreeSwitch {
            parameters:
                role = "aggregation";
                minNodeId = int(index / 2) * 4;
                maxNodeId = int(index / 2) * 4 + 3;
                linkBitrate = parent.linkBitrate;
                @display("p=110,210,row,130");
        }
        edge[8]: FatTreeSwitch {
            parameters:
                role = "edge";
                minNodeId = index * 2;
                maxNodeId = index * 2 + 1;
                linkBitrate = parent.linkBitrate;
                @display("p=110,360,row,130");
        }
        host[16]: ScatterNode {
            parameters:
                nodeId = index;
                numNodes = 16;
                rootId = 0;
                algorithm = parent.algorithm;
                payloadBytes = parent.payloadBytes;
                linkBitrate = parent.linkBitrate;
                @display("p=50,520,row,65");
        }

    connections:
        for i=0..15 {
            host[i].out++ --> FatTreeLink { datarate = parent.linkBitrate; } --> edge[int(i / 2)].in++;
            edge[int(i / 2)].out++ --> FatTreeLink { datarate = parent.linkBitrate; } --> host[i].in++;
        }

        for k=0..15 {
            edge[int(k / 2)].out++ --> FatTreeLink { datarate = parent.linkBitrate; } --> agg[int(int(k / 4) * 2 + k % 2)].in++;
            agg[int(int(k / 4) * 2 + k % 2)].out++ --> FatTreeLink { datarate = parent.linkBitrate; } --> edge[int(k / 2)].in++;
        }

        for k=0..31 {
            agg[int(k / 4)].out++ --> FatTreeLink { datarate = parent.linkBitrate; } --> core[k % 4].in++;
            core[k % 4].out++ --> FatTreeLink { datarate = parent.linkBitrate; } --> agg[int(k / 4)].in++;
        }
}

network ScatterFatTree32
{
    parameters:
        int payloadBytes = default(1048576);
        string algorithm = default("mpi_scatter");
        double linkBitrate @unit(bps) = default(1Gbps);
        @display("bgb=1600,720");

    submodules:
        core[8]: FatTreeSwitch {
            parameters:
                role = "core";
                linkBitrate = parent.linkBitrate;
                @display("p=280,80,row,140");
        }
        agg[16]: FatTreeSwitch {
            parameters:
                role = "aggregation";
                minNodeId = int(index / 2) * 4;
                maxNodeId = int(index / 2) * 4 + 3;
                linkBitrate = parent.linkBitrate;
                @display("p=90,220,row,95");
        }
        edge[16]: FatTreeSwitch {
            parameters:
                role = "edge";
                minNodeId = index * 2;
                maxNodeId = index * 2 + 1;
                linkBitrate = parent.linkBitrate;
                @display("p=90,390,row,95");
        }
        host[32]: ScatterNode {
            parameters:
                nodeId = index;
                numNodes = 32;
                rootId = 0;
                algorithm = parent.algorithm;
                payloadBytes = parent.payloadBytes;
                linkBitrate = parent.linkBitrate;
                @display("p=45,610,row,48");
        }

    connections:
        for i=0..31 {
            host[i].out++ --> FatTreeLink { datarate = parent.linkBitrate; } --> edge[int(i / 2)].in++;
            edge[int(i / 2)].out++ --> FatTreeLink { datarate = parent.linkBitrate; } --> host[i].in++;
        }

        for k=0..31 {
            edge[int(k / 2)].out++ --> FatTreeLink { datarate = parent.linkBitrate; } --> agg[int(int(k / 4) * 2 + k % 2)].in++;
            agg[int(int(k / 4) * 2 + k % 2)].out++ --> FatTreeLink { datarate = parent.linkBitrate; } --> edge[int(k / 2)].in++;
        }

        for k=0..127 {
            agg[int(k / 8)].out++ --> FatTreeLink { datarate = parent.linkBitrate; } --> core[k % 8].in++;
            core[k % 8].out++ --> FatTreeLink { datarate = parent.linkBitrate; } --> agg[int(k / 8)].in++;
        }
}
