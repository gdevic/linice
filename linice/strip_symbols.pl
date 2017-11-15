#!/usr/bin/perl

	# This script removes unneeded information from the linice_kernel.o file
	# and renames symbols that are not needed (for the final link).
	#
	# The command line parameter is the location of the linice_kernel.o file
	$linice = pop;

	# Define the list of symbols that are excluded from purge: They are needed to
	# properly link with the iceface.c

	@excluded = qw( DriverOpen IceCleanupModule IceInitModule DriverClose DriverIOCTL LiniceRegisterExtension LiniceUnregisterExtension );

	# Strip the object code from all unnecessary symbols
	system("objcopy --strip-unneeded --discard-all $linice");

	# Now dump the rest of the symbols that remained into the symbol text file
	system("nm $linice > syms.txt");

	# Open that symbol text file and read all the symbols from there
	open(SYMS, "<syms.txt") || die "Cannot open syms.txt!";

	# ordinal specifies the renamed symbol count
	$ordinal = 100;
	printf("Redefining individual symbols...\n");

	$list = "";
	$groupcount = 0;

	while ($line = <SYMS>)	{

		chop $line;

		# The format of the symbol list line is: "00004483 T SymbolName"
		#                                 index:     0    123    4
		@symbols = $line =~ /(\w+(\s+)(\w+)(\s+)(\w+))/;
		$symbol  = @symbols[4];

		$ordinal = $ordinal + 1;
		$excludeit = 0;

		# If the symbol is one from the excluded list, skip it
		foreach ( @excluded ) {
			if ( $symbol eq $_ )
			{
				$excludeit = 1;
			}
		}

		# If the symbol is undefined (has no address), also exclude it
		if ( $symbol eq "" )
		{
			$excludeit = 1;
		}

		if ( $excludeit==0 )
		{
			$list = "$list --redefine-sym $symbol=L$ordinal";

			$groupcount = $groupcount + 1;

			# Since calling objcopy for each symbol would take long time,
			# we group a number of symbol pairs together; the real limit is the
			# command line buffer size...
			if ( $groupcount==100 ) {

				$groupcount = 0;

				printf("objcopy $list $linice\n");
				system("objcopy $list $linice\n");

				$list = "";
			}
		}
	}

	# Flush the rest of the items to redefine if the group is non-empty
	if ( $list ne "" ) {
		printf("objcopy $list $linice\n");
		system("objcopy $list $linice\n");
	}

	close(SYMS);

