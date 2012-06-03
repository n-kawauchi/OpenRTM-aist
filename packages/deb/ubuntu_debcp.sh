#!/bin/sh
#
# @file vine_repo
# @brief apt-rpm repository database creation for VineLinux
# @date $Date$
# @author Noriaki Ando <n-ando@aist.go.jp>
#
# Copyright (C) 2008
#     Noriaki Ando
#     Task-intelligence Research Group,
#     Intelligent Systems Research Institute,
#     National Institute of
#         Advanced Industrial Science and Technology (AIST), Japan
#     All rights reserved.
#
# $Id$
#

# Base directory of repository
basedir="/exports/pub/repository/pub/Linux/ubuntu/dists"
pkgdir="/usr/users/builder/PackageBuild/"
# debian versions
codenames="edgy feisty gutsy hardy intrepid jaunty karmic lucid"
codenames="lucid maverick"
# VineLinux architectures
darchs="i386 amd64"

debcode2ver () {
    case $1 in
	"edgy")
	    echo "6.10"
	    ;;
	"feisty")
	    echo "7.04"
	    ;;
	"gutsy")
	    echo "7.10"
	    ;;
	"hardy")
	    echo "8.04"
	    ;;
	"intrepid")
	    echo "8.10"
	    ;;
	"jaunty")
	    echo "9.04"
	    ;;
	"karmic")
	    echo "9.10"
	    ;;
	"lucid")
	    echo "10.04LTS"
	    ;;
	"maverick")
	    echo "10.10"
	    ;;
	*)
	    echo "unknown code name"
	    exit 1
	    ;;
    esac
}

debarch2arch () {
    case $1 in
	"i386")
	    echo "i686"
	    ;;
	"amd64")
	    echo "x86_64"
	    ;;
	*)
	    echo "unknown arch"
	    exit 1
	    ;;
    esac
}



for codename in $codenames; do
    for darch in $darchs ; do
	version=`debcode2ver $codename`
	arch=`debarch2arch $darch`

	debs="$pkgdir/Ubuntu$version-$arch/"
	repo="$basedir/$codename/main/binary-$darch"

	# ディレクトリが存在しないならスキップ
	if test ! -d $debs ; then
	    echo $debs "does not exists. skiped."
	    continue
	fi

	# ディレクトリが存在しないなら作成
	if test ! -d $repo ; then
	    mkdir -p $repo
	fi

	echo ""
	echo "Copying debs to repository:"
	echo "dir:  "$repo

	for deb in $debs* ; do
	    debname=`basename $deb`
	    if test -f $repo/$debname ; then
		echo "$repo/$debname already exists"
		read -p "Overwrite? $rpmname [Y/n]" ow
		if test ! "$ow" = "n" ; then
		    echo "copying $debname"
		    cp $debs/$debname $repo
		    echo "copy done"
		fi
	    else
		cp $debs/$debname $repo
		echo "copy done"
	    fi
	done
    done
done
