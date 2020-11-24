#! /bin/bash

# How to Build Environment:
# step1: sudo apt-get install doxygen
# step2: sudo apt-get install xdot
# step2: sudo apt-get install texlive-full
# step3: sudo apt-get install latex-cjk-chinese*
# step4: sudo apt-get install cjk-latex
# step5: run this shell
#
# run doxygen and creat html and latex, change latex refman.tex
# to enable it support chinese, the use texlive to make pdf

#sed -i '/PROJECT_NAME    /cPROJECT_NAME = "Copyright (C) 2019 Stream Computing Inc."' ./Doxyfile
#sed -i '/FILE_PATTERNS    /cFILE_PATTERNS = "eigen3_ops.cc eigen3_ops.h"' ./Doxyfile
#sed -i '/OUTPUT_LANGUAGE    /cOUTPUT_LANGUAGE = Chinese' ./Doxyfile
#sed -i '/DOT_GRAPH_MAX_NODES   /cDOT_GRAPH_MAX_NODES = 500' ./Doxyfile
#sed -i '819,861d' ./Doxyfile
#sed -i '/LATEX_HEADER    /cLATEX_HEADER = "Copyright (C) 2019 Stream Computing Inc."' ./Doxyfile
#sed -i '/LATEX_FOOTER    /cLATEX_FOOTER = "Copyright (C) 2019 Stream Computing Inc."' ./Doxyfile

doxygen doxygen_eigen3ops.cfg
sed -i -e 's,begin{document},usepackage{CJKutf8}\n\\begin{document}\n\\begin{CJK}{UTF8}{gbsn},' ./latex/refman.tex
sed -i -e 's,end{document},end{CJK}\n\\end{document},' ./latex/refman.tex
sed -i -e 's/Doxygen 1.8.13/hao.chen/' ./latex/refman.tex
sed -i -e 's/制作者 Doxygen/Copyright (C) 2019 Stream Computing Inc./' ./latex/refman.tex

cd ./latex/; make
cp refman.pdf ../eigen3_ops.pdf


