# run doxygen and creat html and latex, change latex refman.tex
# to enable it support chinese, the use texlive to make pdf
doxygen doxygen_eigen3ops.cfg
sed -i -e 's,begin{document},usepackage{CJKutf8}\n\\begin{document}\n\\begin{CJK}{UTF8}{gbsn},' ./latex/refman.tex
sed -i -e 's,end{document},end{CJK}\n\\end{document},' ./latex/refman.tex
#cd ./latex/; make

