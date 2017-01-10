#!/bin/awk

BEGIN {
	print "static char *err[] = {"
}

{
	error=""
	comment=""

	error=$2
	match($0, /\/\*.*\*\//)
	comment=substr($0, RSTART, RLENGTH)
	gsub(/\/\* /, "", comment)
	gsub(/ \*\//, "", comment)

	print "\t[" error "] = \"" comment "\","
}

END {
	print "};"
}
