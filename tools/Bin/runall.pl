#!/usr/bin/perl

#
# Reads a file "files.txt" that contain reference to Linux programs
# to run symbol utility and symbol checker
#

my $progs = "files.txt";

open( INPUT, "<$progs") or die "ERROR: Can't open input file $progs\n";

# Read in one line at a time from the list of files specified in files.txt
while ( <INPUT> )
{
    # Get the next line
    my $line = $_;
	chop($line);

	system("elf $line");
	system("move log.txt $line.log.txt");
	system("chksym $line.sym > $line.sym.txt");
}

printf("\nDONE.\n");