#!/bin/ksh

# merge repos between architectures (sparc and i386)

export LC_ALL=C

date

if [ -z "$PKGDEST" ]
then
	REPOTOP="`cd ../pkgdest; pwd`"
else
	REPOTOP="$PKGDEST/.."
fi

[ ! -z "$REPOTOP_I386" ]	|| REPOTOP_I386=$REPOTOP/i386
[ ! -z "$REPOTOP_SPARC" ]	|| REPOTOP_SPARC=$REPOTOP/sparc
[ ! -z "$REPOTOP_MERGED" ]	|| REPOTOP_MERGED=$REPOTOP/merged

merge_one_pair()
{
	reponame=$1
	publisher=$2

	srs="$REPOTOP_SPARC/$reponame"
	srx="$REPOTOP_I386/$reponame"
	drd="$REPOTOP_MERGED/$reponame"

	rm -rf $drd
	pkgrepo create $drd
	pkgrepo set -s $drd publisher/prefix=$publisher

	pkgmerge -d $drd -s arch=sparc,$srs -s arch=i386,$srx

	/usr/lib/pkg.depotd -d $drd --add-content --exit-ready
}

REPONAME="repo.l10n"
PUBLISHER=${L10N_PUBLISHER:-"l10n-nightly"}

merge_one_pair $REPONAME $PUBLISHER

date
