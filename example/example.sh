#!/bin/sh

mkdir target
env TDFS_DEBUG_SET_YEAR=2014 TDFS_DEBUG_SET_MONTH=07 ../src/tdfs ./workroot ./target

echo 'You have an old diary file'
echo '% find workroot'
find workroot
sleep 1
echo 'You can find a diary on the target'
echo '% cat target/my_diary.txt'
cat target/my_diary.txt
sleep 1
echo ''
echo 'Then you can update it.'
cat << __EOF >> target/my_diary.txt

* 2014-07-12

I find I cannot write a daily diary.

__EOF
cat target/my_diary.txt
sleep 1
echo 'Then your diary will brought to newest workdir'
echo '(see the 2014/07 directory)'
echo '% find workroot'
find workroot
sleep 1

echo 'I use svn to manage workroot, and checkout top N months on the go to save disk space.'
echo 'With TDFS I can keep my important files with me! :-)'


