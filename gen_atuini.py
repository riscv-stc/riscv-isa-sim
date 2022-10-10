#!/usr/bin/python3

import os
import sys
import getopt

# unified mode of 4 clusters in vf_0

npc_bases = [
    # cluster 0
    [
        0xdc000000, 0xd8000000, 0xd4000000, 0xd4800000,
        0xd5000000, 0xd5800000, 0xd8800000, 0xdc800000,
    ],

    # cluster 1
    [
        0xec000000, 0xe8000000, 0xe4000000, 0xe4800000,
        0xe5000000, 0xe5800000, 0xe8800000, 0xec800000,
    ],

    # cluster 2
    [
        0xc8000000, 0xc8800000, 0xca000000, 0xca800000,
        0xcc000000, 0xcc800000, 0xce000000, 0xce800000,
    ],

    # cluster 3
    [
        0xf0000000, 0xf0800000, 0xf2000000, 0xf2800000,
        0xf4000000, 0xf4800000, 0xf6000000, 0xf6800000,
    ],
]

sysdma_bases = [
    [0xd2f00000, 0xd3f00000],
    [0xe2f00000, 0xe3f00000],
    [0xc2f00000, 0xc3f00000],
    [0xfaf00000, 0xfbf00000],
]

at_npc_entries = [
    # [0] DDR_FW, 2MB per cluster
    {"pa.base": 0x0000000000, "ipa.start": 0x0000000000, "ipa.end": 0x0000200000},

    # [1] task_space, 128MB
    {"pa.base": 0x08e0000000, "ipa.start": 0x0060000000, "ipa.end": 0x0068000000},

    # [2] IRAM, 512KB
    {"pa.base": 0x00d3d80000, "ipa.start": 0x00cd000000, "ipa.end": 0x00cd080000},
    # [3] IRAM_LOG, 2KB per npc
    {"pa.base": 0x0000000000, "ipa.start": 0x00ce000000, "ipa.end": 0x00ce000800},

    # [4] sysdma0, 16KB
    {"pa.base": 0x0000000000, "ipa.start": 0x00d0000000, "ipa.end": 0x00d0004000},
    # [5] sysdma1, 16KB
    {"pa.base": 0x0000000000, "ipa.start": 0x00d0004000, "ipa.end": 0x00d0008000},

    # [6] LLB_P0 shared, 40MB
    {"pa.base": 0x00d9000000, "ipa.start": 0x00d9000000, "ipa.end": 0x00db800000},
    # [7] LLB_P1 shared, 40MB
    {"pa.base": 0x00e9000000, "ipa.start": 0x00e9000000, "ipa.end": 0x00eb800000},
    # [8] LLB_P0 local, 10MB. optional
    {"pa.base": 0x0000000000, "ipa.start": 0x00c9000000, "ipa.end": 0x00c9a00000},
    # [9] LLB_P1 local, 10MB. optional
    {"pa.base": 0x0000000000, "ipa.start": 0x00c9a00000, "ipa.end": 0x00ca400000},

    # [10] stack, 256KB per npc
    {"pa.base": 0x0000000000, "ipa.start": 0x00fffc0000, "ipa.end": 0x0100000000},

    # [11] extended user space, 4KB
    {"pa.base": 0x0000000000, "ipa.start": 0x007ffff000, "ipa.end": 0x0080000000},

    # [12] DDR_APP_0, 3GB - 4MB
    {"pa.base": 0x0800400000, "ipa.start": 0x0100000000, "ipa.end": 0x01bfc00000},
    # [13] DDR_APP_1, 4GB - 4MB
    {"pa.base": 0x0900400000, "ipa.start": 0x01bfc00000, "ipa.end": 0x02bf800000},
    # [14] DDR_APP_2, 4GB - 4MB
    {"pa.base": 0x0a00400000, "ipa.start": 0x02bf800000, "ipa.end": 0x03bf400000},
    # [15] DDR_APP_3, 4GB - 4MB
    {"pa.base": 0x0b00400000, "ipa.start": 0x03bf400000, "ipa.end": 0x04bf000000},
]

# MTE doesn't support biased addresses other than 0xd9/0xe9
at_mte_entries = [
    # [0] LLB_P0 shared, 40MB
    {"pa.base": 0x00d9000000, "ipa.start": 0x00d9000000, "ipa.end": 0x00db800000},
    # [1] LLB_P1 shared, 40MB
    {"pa.base": 0x00e9000000, "ipa.start": 0x00e9000000, "ipa.end": 0x00eb800000},
]

at_sysdma_entries = [
    # [0] LLB_P0 shared, 40MB
    {"pa.base": 0x00d9000000, "ipa.start": 0x00d9000000, "ipa.end": 0x00db800000},
    # [1] LLB_P1 shared, 40MB
    {"pa.base": 0x00e9000000, "ipa.start": 0x00e9000000, "ipa.end": 0x00eb800000},
    # [2] LLB_P0 local, 10MB. optional
    {"pa.base": 0x00d9000000, "ipa.start": 0x00c9000000, "ipa.end": 0x00c9a00000},
    # [3] LLB_P1 local, 10MB. optional
    {"pa.base": 0x00e9000000, "ipa.start": 0x00c9a00000, "ipa.end": 0x00ca400000},

    # [4-11] L1 buffers, (1024+288)KB for each npc. optional
    {"pa.base": 0x0000000000, "ipa.start": 0x00c0000000, "ipa.end": 0x00c0148000},
    {"pa.base": 0x0000000000, "ipa.start": 0x00c1000000, "ipa.end": 0x00c1148000},
    {"pa.base": 0x0000000000, "ipa.start": 0x00c2000000, "ipa.end": 0x00c2148000},
    {"pa.base": 0x0000000000, "ipa.start": 0x00c3000000, "ipa.end": 0x00c3148000},
    {"pa.base": 0x0000000000, "ipa.start": 0x00c4000000, "ipa.end": 0x00c4148000},
    {"pa.base": 0x0000000000, "ipa.start": 0x00c5000000, "ipa.end": 0x00c5148000},
    {"pa.base": 0x0000000000, "ipa.start": 0x00c6000000, "ipa.end": 0x00c6148000},
    {"pa.base": 0x0000000000, "ipa.start": 0x00c7000000, "ipa.end": 0x00c7148000},

    # [12] DDR_APP_0, 3GB - 4MB
    {"pa.base": 0x0800400000, "ipa.start": 0x0100000000, "ipa.end": 0x01bfc00000},
    # [13] DDR_APP_1, 4GB - 4MB
    {"pa.base": 0x0900400000, "ipa.start": 0x01bfc00000, "ipa.end": 0x02bf800000},
    # [14] DDR_APP_2, 4GB - 4MB
    {"pa.base": 0x0a00400000, "ipa.start": 0x02bf800000, "ipa.end": 0x03bf400000},
    # [15] DDR_APP_3, 4GB - 4MB
    {"pa.base": 0x0b00400000, "ipa.start": 0x03bf400000, "ipa.end": 0x04bf000000},
]

at_smmuptw_entries = [
    # [0] DDR_FW, 2MB
    {"pa.base": 0x0800000000, "ipa.start": 0x0000000000, "ipa.end": 0x0000200000},

    # [1] L1 buffer, (1024+288)KB on npc0 of each cluster
    {"pa.base": 0x0000000000, "ipa.start": 0x00c0000000, "ipa.end": 0x00c0148000},

    # [2] DDR_APP_0, 3GB - 4MB
    {"pa.base": 0x0800400000, "ipa.start": 0x0100000000, "ipa.end": 0x01bfc00000},
    # [3] DDR_APP_1, 4GB - 4MB
    {"pa.base": 0x0900400000, "ipa.start": 0x01bfc00000, "ipa.end": 0x02bf800000},
    # [4] DDR_APP_2, 4GB - 4MB
    {"pa.base": 0x0a00400000, "ipa.start": 0x02bf800000, "ipa.end": 0x03bf400000},
    # [5] DDR_APP_3, 4GB - 4MB
    {"pa.base": 0x0b00400000, "ipa.start": 0x03bf400000, "ipa.end": 0x04bf000000},
]

def usage(cmd):
    print("%s usage:" %cmd)
    print("  -c, --cores    number of cores. default: 32")
    print("  -d, --dies     number of dies. default: 1")
    print("  -p, --pcie     generate pcie only AT entries")
    print("  -o, --out      filename of the generated ini. default: atu.ini")
    return

if __name__ == '__main__':
    nr_dies = 1
    nr_cores = 32
    outfile = "atu.ini"
    pcie_only = 0

    opts, args = getopt.getopt(sys.argv[1:], \
            "-h-d:-c:-p-o", \
            ["help", "dies", "cores", "pcie", "out"] )

    for opt, optarg in opts:
        if opt in ("-h", "--help"):
            usage(sys.argv[0] )
            exit(-1)

        if opt in ("-d", "--dies"):
            try:
                nr_dies = int(optarg)
                if nr_dies > 1:
                    print("multi-die not supported yet: %d" %nr_dies)
            except:
                print("invalid die number: %s" %optarg)
                usage(sys.argv[0] )
                exit(-1)

        if opt in ("-c", "--cores"):
            try:
                nr_cores = int(optarg)
            except:
                print("invalid core number: %s" %optarg)
                usage(sys.argv[0] )
                exit(-1)

        if opt in ("-p", "--pcie"):
            pcie_only = 1
            outfile = "atu-pcie.ini"

        if opt in ("-o", "--out"):
            outfile = optarg

    print("generating %s for nr_dies: %d, nr_cores: %d ..." %(outfile, nr_dies, nr_cores) )

    output = open(outfile, "w+")

    for core_id in range(nr_cores):
        if core_id < 8:
            cluster_id = 0
        elif core_id < 16:
            cluster_id = 1
        elif core_id < 24:
            cluster_id = 2
        else:
            cluster_id = 3

        # ---- at_npc_entries ----
        # DDR_FW, 2MB
        at_npc_entries[0]['pa.base'] = 0x0800000000 + cluster_id * 0x100000000

        # IRAM_LOG, 2KB
        at_npc_entries[3]['pa.base'] = 0x00d3d80000 + cluster_id * 0x20000 + 0x1c000 + (core_id & 0x7) * 0x800

        # sysdma0, 16KB
        at_npc_entries[4]['pa.base'] = sysdma_bases[cluster_id][0]
        # sysdma1, 16KB
        at_npc_entries[5]['pa.base'] = sysdma_bases[cluster_id][1]

        # LLB_P0 local
        at_npc_entries[8]['pa.base'] = 0x00d9000000 + cluster_id * 0xa00000
        # LLB_P1 local
        at_npc_entries[9]['pa.base'] = 0x00e9000000 + cluster_id * 0xa00000

        # stack, 256KB
        at_npc_entries[10]['pa.base'] = 0x0800200000 + cluster_id * 0x100000000 + (core_id & 0x7) * 0x40000

        # extended user space, 4KB
        at_npc_entries[11]['pa.base'] = 0x08001ff000 + cluster_id * 0x100000000

        section = "[ipa-trans-core%d]\n" %core_id
        output.write(section)

        for i in range(len(at_npc_entries)):
            for key, value in at_npc_entries[i].items():
                item = "entry%d.%s=0x%010x\n" %(i, key, value)
                output.write(item)

        # ---- at_mte_entries ----
        section = "[ipa-trans-mte-core%d]\n" %core_id
        output.write(section)

        for i in range(len(at_mte_entries)):
            for key, value in at_mte_entries[i].items():
                item = "entry%d.%s=0x%010x\n" %(i, key, value)
                output.write(item)

    for sysdma_id in range(8):
        if sysdma_id < 2:
            cluster_id = 0
        elif sysdma_id < 4:
            cluster_id = 1
        elif sysdma_id < 6:
            cluster_id = 2
        else:
            cluster_id = 3

        # ---- at_sysdma_entries ----
        # LLB_P0 local
        at_sysdma_entries[2]['pa.base'] = 0x00d9000000 + cluster_id * 0xa00000
        # LLB_P1 local
        at_sysdma_entries[3]['pa.base'] = 0x00e9000000 + cluster_id * 0xa00000

        for i in range(4, 12):
            core_id = i - 4;
            at_sysdma_entries[i]['pa.base'] = npc_bases[cluster_id][core_id]

        for ch_id in range(2):
            section = "[ipa-trans-sysdma%d-ch%d]\n" %(sysdma_id, ch_id)
            output.write(section)

            for i in range(len(at_sysdma_entries)):
                for key, value in at_sysdma_entries[i].items():
                    item = "entry%d.%s=0x%010x\n" %(i, key, value)
                    output.write(item)

    # ---- at_smmuptw_entries ----
    # XXX: not applicable for spike

    output.close()
    print("done.")
