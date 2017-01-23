#!/usr/bin/awk

BEGIN {
	print "#include \"sterror.h\""
	print ""
	print "static char *err_str[] = {"
}

/^$/ {
	next
}

/^[[:space:]]+E/ {
	error=""
	msg=""

	error=$1
	gsub(/^[[:space:]]+E[A-Z]+[[:space:]]+/, "")
	gsub(/\(.+\)/, "")
	msg=$0

	print "\t[" error "] = \"" msg "\","
}

END {
	print "};"
	print ""
	print "char	*strerror(int err_num)"
	print "{"
	print "\tif (err_num < 0 || err_num > EARGS)"
	print "\t\treturn ((void *)0);"
	print "\treturn (err_str[err_num]);"
	print "}"
}
