#!/bin/bash
DESTDIR="/home/ubuntu/workspace//non-web/Uzebox/MARIOBROS2018/"

# Should chdir into the ./C2BIN dir.

# DELETE OLD FILES.
echo Deleting old files...
rm -f BINS/SD_MUSIC.DAT
rm -f BINS/SPIRAMSR.BIN
rm -f BINS/MBRS2018.BIN
echo Done...
echo

# Music generation.
# Convert the midi into .inc.
# ../MUSIC/midiconv -s 1 -e 510  ../MUSIC/org_midi/bb01_introfirstlevel.mid ../MUSIC/midiconv_inc/bb01_introfirstlevel.inc # -d
# ../MUSIC/midiconv -s 1 -e 113  ../MUSIC/org_midi/bb02_hurry.mid           ../MUSIC/midiconv_inc/bb02_hurry.inc           # -d
# ../MUSIC/midiconv -s 1 -e 2684 ../MUSIC/org_midi/bb03_maintheme.mid       ../MUSIC/midiconv_inc/bb03_maintheme.inc       # -d
# ../MUSIC/midiconv -s 1 -e 2309 ../MUSIC/org_midi/bb04_mainthemefast.mid   ../MUSIC/midiconv_inc/bb04_mainthemefast.inc   # -d
# ../MUSIC/midiconv -s 1 -e 300  ../MUSIC/org_midi/bb05_titleintro.mid      ../MUSIC/midiconv_inc/bb05_titleintro.inc      # -d -e 375
# ../MUSIC/midiconv -s 1 -e 344  ../MUSIC/org_midi/bb06_invincible.mid      ../MUSIC/midiconv_inc/bb06_invincible.inc      # -d
# ../MUSIC/midiconv -s 1 -e 134  ../MUSIC/org_midi/bb07_theyrehere.mid      ../MUSIC/midiconv_inc/bb07_theyrehere.inc      # -d
# ../MUSIC/midiconv -s 1 -e 582  ../MUSIC/org_midi/bb08_thankyou.mid        ../MUSIC/midiconv_inc/bb08_thankyou.inc        # -d -e 585
# ../MUSIC/midiconv -s 1 -e 811  ../MUSIC/org_midi/bb09_finalboss.mid       ../MUSIC/midiconv_inc/bb09_finalboss.inc       # -d
# ../MUSIC/midiconv -s 1 -e 1601 ../MUSIC/org_midi/bb10_badend.mid          ../MUSIC/midiconv_inc/bb10_badend.inc          # -d
# ../MUSIC/midiconv -s 1 -e 5391 ../MUSIC/org_midi/bb11_credits.mid         ../MUSIC/midiconv_inc/bb11_credits.inc         # -d
# ../MUSIC/midiconv -s 1 -e 959  ../MUSIC/org_midi/bb12_happyend.mid        ../MUSIC/midiconv_inc/bb12_happyend.inc        # -d
# ../MUSIC/midiconv -s 1 -e 214  ../MUSIC/org_midi/bb13_victory.mid         ../MUSIC/midiconv_inc/bb13_victory.inc         # -d
# ../MUSIC/midiconv -s 1 -e 4608 ../MUSIC/org_midi/bb14_secretroom.mid      ../MUSIC/midiconv_inc/bb14_secretroom.inc      # -d
# ../MUSIC/midiconv -s 1 -e 145  ../MUSIC/org_midi/bb15_gameover.mid        ../MUSIC/midiconv_inc/bb15_gameover.inc        # -d

# Convert the .inc to a compressed .inc.
 # ../MUSIC/mconvert ../MUSIC/sdImageConfig.cfg

# Get the size of the music data.
file=./BINS/SD_MUSIC.DAT
if [ -e "$file" ]; then
    echo "File exists"
    size1="$(wc -c < BINS/SD_MUSIC.DAT)"
else
    echo "File does not exist"
    size1=0
fi

echo Size of SD_MUSIC.DAT = $size1
#  # Copy the resulting filesize to the end of a defines file.
#  # Copy files to their intended destinations.
#  # Suppress exit code and error messages
#
#  # Game resource generation. Pass in the size of the music data.
echo -_-_-_-
echo $size
 gcc c2bin.c -std=gnu99 && ./a.out $size1
 rm a.out
echo -_-_-_-

# Get size of C2BIN output.
size2="$(wc -c < BINS/SPIRAMSR.BIN)"

# Combine C2BIN output with the music data.
# cat BINS/SPIRAMSR.BIN BINS/SD_MUSIC.DAT > BINS/MBRS2018.BIN
cat BINS/SD_MUSIC.DAT BINS/SPIRAMSR.BIN > BINS/MBRS2018.BIN

# Get the size of the combined data.
size3="$(wc -c < BINS/MBRS2018.BIN)"

# Output the file sizes.
echo
echo Size of: SD_MUSIC.DAT: $size1
echo Size of: SPIRAMSR.BIN: $size2
echo Size of: MBRS2018.BIN: $size3 \(combined files\)
remain=$((131072 - size3))
echo Remaining bytes: $remain

# Copy the binary resource files to the game directory.
cp -f BINS/MBRS2018.BIN $DESTDIR/output/
cp -f BINS/defineSR.def $DESTDIR/src/assets

# Copy the inc resource files to the game directory.
 cp -f ../PROGMEM/*.inc $DESTDIR/src/assets

# echo // C 2 Bin - DONE!