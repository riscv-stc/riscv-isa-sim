# for eew in (8, 16, 32, 64):
#     for dim in ('a', 'b'):
#         r_name = "ml" + dim + 'e' + str(eew) + "_m" + ".h"
#         with open(r_name, 'w') as f:
#             f.write("MTU_TR_LD(" + str(0) + ", '" + dim + "', uint" + str(eew)+ ");")
#         c_name = "ml" + dim + 'te' + str(eew) + "_m" + ".h"
#         with open(c_name, 'w') as f:
#             f.write("MTU_TR_LD(" + str(1) + ", '" + dim + "', uint" + str(eew)+ ");")
        
#         r_name = "ms" + dim + 'e' + str(eew) + "_m" + ".h"
#         with open(r_name, 'w') as f:
#             f.write("MTU_TR_ST(" + str(0) + ", '" + dim + "', uint" + str(eew)+ ");")
#         c_name = "ms" + dim + 'te' + str(eew) + "_m" + ".h"
#         with open(c_name, 'w') as f:
#             f.write("MTU_TR_ST(" + str(1) + ", '" + dim + "', uint" + str(eew)+ ");")

#     dim = 'c'
#     r_name = "ml" + dim + 'e' + str(eew) + "_m" + ".h"
#     with open(r_name, 'w') as f:
#         f.write("MTU_ACC_LD(" + str(0) + ", '" + dim + "', uint" + str(eew)+ ");")
#     c_name = "ml" + dim + 'te' + str(eew) + "_m" + ".h"
#     with open(c_name, 'w') as f:
#         f.write("MTU_ACC_LD(" + str(1) + ", '" + dim + "', uint" + str(eew)+ ");")

#     r_name = "ms" + dim + 'e' + str(eew) + "_m" + ".h"
#     with open(r_name, 'w') as f:
#         f.write("MTU_ACC_ST(" + str(0) + ", '" + dim + "', uint" + str(eew)+ ");")
#     c_name = "ms" + dim + 'te' + str(eew) + "_m" + ".h"
#     with open(c_name, 'w') as f:
#         f.write("MTU_ACC_ST(" + str(1) + ", '" + dim + "', uint" + str(eew)+ ");")

# for eew in (8, 16, 32, 64):
#     for dim in ('a', 'b', 'c'):
#         r_name = "ml" + dim + "e" + str(eew) + "_v.h"
#         with open(r_name, "w") as f:
#             f.write("MTU_VECTOR_LD(0, '" + dim + "', uint" + str(eew) + ");");

#         r_name = "ms" + dim + "e" + str(eew) + "_v.h"
#         with open(r_name, "w") as f:
#             f.write("MTU_VECTOR_ST(0, '" + dim + "', uint" + str(eew) + ");");
#   