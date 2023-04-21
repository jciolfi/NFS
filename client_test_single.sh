#!/bin/bash

# make sure arg passed is a nonnegative integer.
if [ $# -ne 1 ] || ! [[ "$1" =~ ^[0-9]+$ ]]; then
    echo "Error: Must supply sleep time as a nonnegative integer between fget commands"
    exit 1
fi

# print newline and sleep after each command.
sleep_time=$1
end_fget() {
    echo ""
    sleep $sleep_time
}

echo "ⓘ ⓧ pass no arguments to fget"
./fget
end_fget

echo "ⓘ ⓧ pass invalid action 'MD1' to fget"
./fget MD1 dir1
end_fget

echo "ⓘ ✓ MD: Creating directory 'dir1'"
./fget MD dir1
end_fget

echo "ⓘ ⓧ MD: Creating directory with same name 'dir1' should not succeed"
./fget MD dir1
end_fget

echo "ⓘ ✓ MD: Creating directory 'dir1/tempdir'"
./fget MD dir1/tempdir
end_fget

echo "ⓘ ⓧ MD: Creating a directory with an invalid path 'dir1/tempdir/empty.txt/baddir' should not succeed"
./fget MD dir1/tempdir/empty.txt/baddir
end_fget

echo "ⓘ ✓ PUT: Putting empty.txt file 'dir1/tempdir/empty.txt'"
./fget PUT empty.txt dir1/tempdir/empty.txt
end_fget

echo "ⓘ ⓧ RM: removing file 'badfile.txt' that doesn't exist"
./fget RM badfile.txt
end_fget

echo "ⓘ ✓ RM: remove file 'dir1/tempdir/empty.txt'"
./fget RM dir1/tempdir/empty.txt
end_fget

echo "ⓘ ✓ RM: remove empty directory 'dir1/tempdir'"
./fget RM dir1/tempdir
end_fget

echo "ⓘ ⓧ PUT: put file 'badfile.txt' that doesn't exist locally"
./fget PUT badfile.txt
end_fget

echo "ⓘ ✓ PUT: put file 'file1.txt' without specifying remote name"
./fget PUT file1.txt
end_fget

echo "ⓘ ✓ PUT: put file 'file1.txt' into dir1 with specifying remote name as 'newfile.txt'"
./fget PUT file1.txt dir1/newfile.txt
end_fget

echo "ⓘ ✓ PUT: put non-txt file fget without specifying remote name"
./fget PUT Makefile
end_fget

echo "ⓘ ⓧ INFO: get info for file 'badfile.abc' that doesn't exist"
./fget INFO badfile.abc
end_fget

echo "ⓘ ✓ INFO: get info for file1.txt"
./fget INFO file1.txt
end_fget

echo "ⓘ ✓ INFO: get info for Makefile"
./fget INFO file1.txt
end_fget

echo "ⓘ ⓧ GET: get file 'badfile.xyz' that doesn't exist"
./fget GET badfile.xyz
end_fget

echo "Custom: removing file1.txt so it can be retrieved from fget"
rm file1.txt
echo "ⓘ ✓ GET: get file1.txt without specifying local path"
./fget GET file1.txt
end_fget

echo "ⓘ ✓ GET: get file1.txt with specifying local path as temp_file.txt"
./fget GET file1.txt temp_file.txt
end_fget

echo "ⓘ ✓ GET: get Makefile with specifying local path as Makefile_copy"
./fget GET Makefile Makefile_copy
end_fget

echo "ⓘ ✓ RM: remove Makefile"
./fget RM Makefile
end_fget

echo "ⓘ ✓ RM: remove dir1/newfile.txt"
./fget RM dir1/newfile.txt
end_fget

echo "ⓘ ✓ RM: remove dir1/"
./fget RM dir1
end_fget