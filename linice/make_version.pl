##############################################################################
#																			 #
#	Generate a version file in the bin directory							 #
#																			 #
##############################################################################

open(ICEVER, "<../include/ice-version.h");// || die "Can't open ice-version.h!\n";

while ( <ICEVER> )
{
	chomp;
    my $line = $_;
	my @tokens = split(/\s* /, $line);

	if($tokens[0] eq "#define")
	{
		if($tokens[1] eq "MAJOR_VERSION")
		{
			$major = $tokens[2];
		}
		if($tokens[1] eq "MINOR_VERSION")
		{
			$minor = $tokens[2];
		}
	}
}

open(VER, ">../bin/Version.txt") || die "Can't create ../bin/Version.txt!\n";

# Generate the version number information

printf(VER "Linice version $major.$minor\n");

# Generate the date / time stamp

($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst)=localtime(time);
printf(VER "Compiled on %4d-%02d-%02d %02d:%02d:%02d\n",
	$year+1900,$mon+1,$mday,$hour,$min,$sec);

close(VER);
