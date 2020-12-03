#!/bin/bash

if [ $# -ne 2 ]
then
	echo "Usage: $0 <directory_to_package> <output_file>"
	exit 1
fi

directory=$1
script=`readlink -f $2`

cat > ${script} <<EOF
#!/bin/bash

if [ \`basename \$0\` = 'bash' ] || [ \`basename \$0\` = 'sh' ]; then
	cat > /tmp/.XXXinstall
        bash /tmp/.XXXinstall
	exit
fi

[ -n "\$RISCV" ] || RISCV=\$HOME/opt/riscv
mkdir -p \$RISCV

echo -n "Install to \$RISCV..."
start_line_of_tar=\$((\`grep -an "^__ARCHIVE_BELOW__$" \$0 | cut -d: -f1\` + 1))
tail -n+\${start_line_of_tar} \$0 | tar zxf - -C \$RISCV
echo "Done."
exit 0
__ARCHIVE_BELOW__
EOF

cd ${directory} && tar zcf - . >> ${script}

chmod +x ${script}
