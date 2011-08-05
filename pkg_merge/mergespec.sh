#!/bin/ksh

# merge multiple sub-repositories (for same architecture) into single one
# from:
#     repo.all: created with static manifests
#     repo.imspec: created with pkgbuild in inputmethod/specs
#     repo.spec: created with pkgbuild in g11n-spec
#     repo.import: [workaround] created from SVR4 packages
# to:
#     repo.l10n

export LC_ALL=C

date

if [ $# -eq 0 ] || [ \( "$1" != "sparc" \) -a \( "$1" != "i386" \) ]
then
	print -u2 "architecture not specified or invalid value"
	print -u2 "usage: $0 sparc|i386"
	exit 1
else
	PKGMACH=$1
fi

PKGDEST="${PKGDEST:-`cd ../pkgdest/$PKGMACH; pwd`}"
SR_STATIC=file://$PKGDEST/repo.all
SR_IMSPEC=file://$PKGDEST/repo.imspec
SR_SPEC=file://$PKGDEST/repo.spec
SR_IMPORT=file://$PKGDEST/repo.import

DRD=$PKGDEST/repo.l10n
DR=file://$DRD

INC=consolidation/l10n/l10n-incorporation
ALL=consolidation/l10n/l10n-all

PUBLISHER=${L10N_PUBLISHER:-"l10n-nightly"}

rm -rf $DRD
pkgrepo create $DRD
pkgrepo set -s $DR publisher/prefix=${PUBLISHER}

TMPD=$PKGDEST/TMPD.$$
rm -rf $TMPD; mkdir $TMPD
TMPMF=$TMPD/tmpmf

MF_INC=$TMPD/l10n-incorporation.mf
MF_ALL=$TMPD/l10n-all.mf
MF_INC_B=${MF_INC}.body
MF_ALL_B=${MF_ALL}.body
cp /dev/null $MF_INC_B
cp /dev/null $MF_ALL_B

PUBOPTS="--fmri-in-manifest --no-catalog --no-index"
PUBLISH="pkgsend -s $DR publish $PUBOPTS"
LEGACY_REV_DATE=${LEGACY_REV_DATE:-"2010.12.06"}

for sr in $SR_STATIC $SR_IMSPEC $SR_SPEC $SR_IMPORT
do
	echo "receiving from $sr..."
	list="List.`echo $sr | sed -e 's;.*/;;'`"

	pkgrecv -s $sr --newest > $TMPD/$list

	cat $TMPD/$list | while read p
	do
		pkgrecv -s $sr -d $TMPD --raw "$p"
		ps="`echo $p | sed -e 's;^pkg:/;;' -e s:^/$PUBLISHER/::`"
		pd="`echo $ps | sed \
			-e 's;/;%2F;g' -e 's;,;%2C;g' -e 's;:;%3A;g' \
			-e 's;@;/;g'`"
		mf="$TMPD/$pd/manifest"
		fmri="`echo $ps | sed -e 's/:20[0-9][0-9].*//'`"

		if [ "$sr" != "$SR_STATIC" ] && expr "$fmri" : "SUNW" > /dev/null
		then
			echo "[skip] $fmri"
		elif expr "$fmri" : "entire" > /dev/null
		then
			echo "[skip] $fmri"
		elif expr "$fmri" : "$INC" > /dev/null
		then
			if [ "$sr" = "$SR_STATIC" ]
			then
				echo "[l10n-incorporation] $fmri"
				grep -v '^depend ' $mf > $MF_INC
			else
				echo "[skip] $fmri"
			fi
		elif expr "$fmri" : "$ALL" > /dev/null
		then
			echo "[l10n-all] $fmri"
			grep -v '^depend ' $mf > $MF_ALL
		elif grep '^set name=pkg\.obsolete value=true' $mf > /dev/null
		then
			echo "[obsolete] $fmri"
			echo "depend fmri=$fmri type=incorporate" >> $MF_INC_B
			$PUBLISH -d $TMPD/$pd $mf
		elif grep '^set name=pkg\.renamed value=true' $mf > /dev/null
		then
			echo "[renamed] $fmri"
			echo "depend fmri=$fmri type=incorporate" >> $MF_INC_B
			$PUBLISH -d $TMPD/$pd $mf
		elif expr "$fmri" : "dummy" > /dev/null
		then
			echo "[dummy] $fmri"
		else
			if grep "^depend fmri=$INC type=require" $mf > /dev/null
			then
				echo "[current] $fmri"
			else
				echo "[current-addinc] $fmri"
				echo "depend fmri=$INC type=require" >> $mf
			fi
			if grep '^legacy.*REV=' $mf > /dev/null
			then
				echo "adjusting legacy REV date"
				cp $mf $TMPMF
				sed -e '/REV=[^ 	]*20[0-9][0-9]\...\...\...\.../s/20[0-9][0-9]\...\...\...\.../'"$LEGACY_REV_DATE"'/' $TMPMF > $mf
			fi
			echo "depend fmri=$fmri type=incorporate" >> $MF_INC_B
			echo "depend fmri=$fmri type=group" >> $MF_ALL_B
			$PUBLISH -d $TMPD/$pd $mf
		fi
	done
done

sort $MF_INC_B >> $MF_INC
$PUBLISH -d $TMPD $MF_INC
sort $MF_ALL_B >> $MF_ALL
$PUBLISH -d $TMPD $MF_ALL

rm -rf $TMPD

echo "updating catalog and index..."
/usr/lib/pkg.depotd -d $DRD --add-content --exit-ready

date
