#!/bin/bash

LOGIN="${1}"
EMAIL="${2}"
FOLDER="${3}"
CREATED="2016/01/01 00:00:21"
UPDATED="2016/01/01 00:00:42"

if [ "${LOGIN}" == "" ]
then
	printf "%s\n" "Error: specify a login (first argument)"
	printf "%s\n" "Usage: bash ./my_header.sh LOGIN EMAIL FOLDER"
	exit 1
fi

if [ "${EMAIL}" == "" ]
then
	printf "%s\n" "Error: specify an email (second argument)"
	printf "%s\n" "Usage: bash ./my_header.sh LOGIN EMAIL FOLDER"
	exit 1
fi

if [ "${FOLDER}" == "" ]
then
	printf "%s\n" "Error: specify a folder to scan (third argument)"
	printf "%s\n" "Usage: bash ./my_header.sh LOGIN EMAIL FOLDER"
	exit 1
fi

run_create_readme()
{
	local INDEX="${1}"
	local TEST="${2}"
	local TEST_NAME="${TEST##*/}"
	local TEST_FULLNAME="${TEST##*spec/}"
	local PARENT_DIR_NAME="${TEST%/*}"
	local README="${TEST}/README.md"

}

run_browse_directory()
{
	local -i INDEX=${1}
	local DIR="${2}"
	local DIR_NAME="${DIR##*/}"
	local DIR_FULLNAME="${DIR##*spec/}"
	local PARENT_DIR_NAME="${DIR%/*}"
	local README="${DIR}/README.md"
	local SUBDIR
	local IMGS=""

	mkdir -p /tmp/my_header

	for SUBDIR in $(ls -1 "${DIR}")
	do
		if [ -d "${DIR}/${SUBDIR}" ]
		then
			run_browse_directory "$(( INDEX + 1 ))" "${DIR}/${SUBDIR}"
		else
			if [[ "${SUBDIR}" =~ \.c$ || "${SUBDIR}" =~ \.h$ ]]
			then
				echo ${SUBDIR}

				awk -v NAMEOFFILE="${SUBDIR}" -v LOGIN="${LOGIN}" -v LOGIN_EMAIL="${LOGIN} <${EMAIL}>" -v CREATED="${CREATED}" -v UPDATED="${UPDATED}" '
					BEGIN {
						DO_UPDATE=1
					}
					NR == 1 && $0 !~ /^\/\* \*\*\*\*/ {
						DO_UPDATE=0
						print "/* ************************************************************************** */"
						print "/*                                                                            */"
						print "/*                                                        :::      ::::::::   */"
						printf "/*   %-47s    :+:      :+:    :+:   */\n", NAMEOFFILE
						print "/*                                                    +:+ +:+         +:+     */"
						printf "/*   By: %-40s   +#+  +:+       +#+        */\n", LOGIN_EMAIL
						print "/*                                                +#+#+#+#+#+   +#+           */"
						printf "/*   Created: %-19s by %-15s   #+#    #+#             */\n", CREATED, LOGIN
						printf "/*   Updated: %-19s by %-15s  ###   ########.fr       */\n", UPDATED, LOGIN
						print "/*                                                                            */"
						print "/* ************************************************************************** */"
						print ""
					}
					DO_UPDATE == 1 && NR == 4 {
						printf "/*   %-47s    :+:      :+:    :+:   */\n", NAMEOFFILE
						next
					}
					DO_UPDATE == 1 && NR == 6 {
						printf "/*   By: %-40s   +#+  +:+       +#+        */\n", LOGIN_EMAIL
						next
					}
					DO_UPDATE == 1 && NR == 8 {
						printf "/*   Created: %-19s by %-15s   #+#    #+#             */\n", CREATED, LOGIN
						next
					}
					DO_UPDATE == 1 && NR == 9 {
						printf "/*   Updated: %-19s by %-15s  ###   ########.fr       */\n", UPDATED, LOGIN
						next
					}
					{
						print
					}
				' \
					"${DIR}/${SUBDIR}" > /tmp/my_header/file
				cat /tmp/my_header/file > "${DIR}/${SUBDIR}"
			fi
		fi
	done
}

run_browse_directory -1 "${FOLDER}"