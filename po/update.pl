#!/usr/bin/perl -w

#  GNOME PO Update Utility.
#  (C) 2000 The Free Software Foundation
#
#  Author(s): Kenneth Christiansen
#
#  GNOME PO Update Utility can use the XML to POT Generator, ui-extract.pl
#  Please distribute it along with this scrips, aswell as desk.po and
#  README.tools.
#
#  Also remember to change $PACKAGE to reflect the package the script is
#  used within.


# Declare global variables
#-------------------------
my $VERSION = "1.5beta4";
my $LANG    = $ARGV[0];
my $PACKAGE = "evolution";

# Always print as the first thing
#--------------------------------
$| = 1;

# Give error if script is run without an argument
#------------------------------------------------
if (! $LANG){
    print "update.pl:  missing file arguments\n";
    print "Try `update.pl --help' for more information.\n";
    exit;
}

# Use the supplied arguments
#---------------------------
if ($LANG=~/^-(.)*/){

    if ("$LANG" eq "--version"   || "$LANG" eq "-V"){
        &Version;
    }
    elsif ($LANG eq "--help"     || "$LANG" eq "-H"){
	&Help;
    }
    elsif ($LANG eq "--dist"     || "$LANG" eq "-D"){
        &Merging;
    }
    elsif ($LANG eq "--pot"      || "$LANG" eq "-P"){

 	# Check for .headerlock file, so the Makefile
        # will not generate the header files twise 
	#--------------------------------------------
	if (-e ".headerlock"){
   	&GeneratePot;
	}else{
        &GenHeaders;
	&GeneratePot;}
        exit;
    }
    elsif ($LANG eq "--headers"  || "$LANG" eq "-S"){
        &GenHeaders;
        exit;
    }
    elsif ($LANG eq "--maintain" || "$LANG" eq "-M"){
        &Maintain;
    }
    else {
        &InvalidOption;
    }

} else {
   
   # Run standard procedure
   #-----------------------
   if(-s "$LANG.po"){
        &GenHeaders; 
	&GeneratePot;
	&Merging;
	&Status;
   }  

   # Report error if the language file supplied
   # to the command line is non-existent
   #-------------------------------------------
   else {
	&NotExisting;       
   }
}

sub Version{

    # Print version information
    #--------------------------
    print "GNOME PO Updater $VERSION\n";
    print "Written by Kenneth Christiansen <kenneth\@gnome.org>, 2000.\n\n";
    print "Copyright (C) 2000 Free Software Foundation, Inc.\n";
    print "This is free software; see the source for copying conditions.  There is NO\n";
    print "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n";
    exit;
}

sub Help{

    # Print usage information
    #------------------------
    print "Usage: ./update.pl [OPTIONS] ...LANGCODE\n";
    print "Updates pot files and merge them with the translations.\n\n";
    print "  -H, --help                   shows this help page\n";
    print "  -P, --pot                    generate the pot file only\n";
    print "  -S, --headers                generate the XML headerfiles in POTFILES.in\n";
    print "  -M, --maintain               search for missing files in POTFILES.in\n";
    print "  -V, --version                shows the version\n";
    print "\nExamples of use:\n";
    print "update.sh --pot    just creates a new pot file from the source\n";
    print "update.sh da       created new pot file and updated the da.po file\n\n";
    print "Report bugs to <kenneth\@gnome.org>.\n";
    exit;
}

sub Maintain{
   
    # Search and fine, all translatable files
    # ---------------------------------------
    $i18nfiles="find ../ -print | egrep '.*\\.(c|y|cc|c++|h|gob)' ";

    open(BUF2, "POTFILES.in") || die "update.pl:  there's no POTFILES.in!!!\n";
    
    print "Searching for missing _(\" \") entries...\n";
    
    open(BUF1, "$i18nfiles|");

    @buf1_1 = <BUF1>;
    @buf1_2 = <BUF2>;

    # Check if we should ignore some found files, when 
    # comparing with POTFILES.in
    #-------------------------------------------------
    if (-s ".potignore"){
        open FILE, ".potignore";
        while (<FILE>) {
            if ($_=~/^[^#]/o){
                push @bup, $_;
            }
        }
        print "Found .potignore: Ignoring files...\n";
        @buf1_2 = (@bup, @buf1_2);
    }

    foreach my $file (@buf1_1){
        open FILE, "<$file";
        while (<FILE>) {
            if ($_=~/_\(\"/o){
                $file = unpack("x3 A*",$file) . "\n";
                push @buf2_1, $file;
                last;
            }
        }
    }

    @buf3_1 = sort (@buf2_1);
    @buf3_2 = sort (@buf1_2);

    my %in2;
    foreach (@buf3_2) {
        $in2{$_} = 1;
    }

    foreach (@buf3_1){
        if (!exists($in2{$_})){
            push @result, $_ 
        }
    }

    # Save file with information about the files missing
    # if any, and give information about this proceedier
    #---------------------------------------------------
    if(@result){
        open OUT, ">missing";
        print OUT @result;
        print "\nHere is the result:\n\n", @result, "\n";
        print "The file \"missing\" has been placed in the current directory.\n";
        print "Files supposed to be ignored should be placed in \".potignore\"\n";
    }

    # If there is nothing to complain about, notice the user
    #-------------------------------------------------------
    else{
        print "\nWell, it's all perfect! Congratulation!\n";
    }         
}

sub InvalidOption{

    # Handle invalid arguments
    #-------------------------
    print "update.pl: invalid option -- $LANG\n";
    print "Try `update.pl --help' for more information.\n";
}
 
sub GenHeaders{

    # Generate the .h header files, so we can allow glade and
    # xml translation support
    #--------------------------------------------------------
    if(-s "ui-extract.pl"){

        print "Found ui-extract.pl script\nRunning ui-extract...\n";

        open FILE, "<POTFILES.in";
        while (<FILE>) {

           # Find .xml.h files in POTFILES.in and generate the
           # files with help from the ui-extract.pl script
           #--------------------------------------------------
           if ($_=~ /(.*)(\.xml)/o){
              $filename = "../$1.xml";
              $xmlfiles="perl \.\/ui-extract.pl --update $filename";
              system($xmlfiles);
           }
      
           # Find .glade.h files in POTFILES.in and generate
           # the files with help from the ui-extract.pl script
           #--------------------------------------------------
#           elsif ($_=~ /(.*)(\.glade)/o){
#              $filename = "../$1.glade";
#              $xmlfiles="perl \.\/ui-extract.pl --update $filename";
#              system($xmlfiles);  
#           }
       }
       close FILE;

       # Create .headerlock file, so the script will know 
       # that we already passed this section. This is required 
       # since the individual sections can be reaced at different
       # times by the Makefile
       #--------------------------------------------------------- 
       system("touch .headerlock");
   }
}

sub GeneratePot{

    # Generate the potfiles from the POTFILES.in file
    #------------------------------------------------

    print "Building the $PACKAGE.pot...\n";

    system ("mv POTFILES.in POTFILES.in.old");    

    open INFILE, "<POTFILES.in.old";
    open OUTFILE, ">POTFILES.in";
    while (<INFILE>) {
        s/\.glade$/\.glade\.h/;
        s/\.xml$/\.xml\.h/;
        print OUTFILE $_;        
    }
    close OUTFILE;
    close INFILE;

    $GETTEXT ="xgettext --default-domain\=$PACKAGE --directory\=\.\."
             ." --add-comments --keyword\=\_ --keyword\=N\_"
             ." --files-from\=\.\/POTFILES\.in ";  
    $GTEST   ="test \! -f $PACKAGE\.po \|\| \( rm -f \.\/$PACKAGE\.pot "
             ."&& mv $PACKAGE\.po \.\/$PACKAGE\.pot \)";

    system($GETTEXT);
    system($GTEST);
    print "Wrote $PACKAGE.pot\n";
    system("mv POTFILES.in.old POTFILES.in");

    # If .headerlock file is found, it means that the potfiles
    # already has been generated. If so delete the generated 
    # .h header files. The reason for this approach with a 
    # file as a marker is due to that the Makefile runs the
    # scripts in turns
    #---------------------------------------------------------

    if(-e ".headerlock"){
        unlink(".headerlock");

        print "Removing generated header (.h) files...";

        open FILE, "<POTFILES.in";
        while (<FILE>) {

           # Delete header files coming from xml files
           #------------------------------------------
           if ($_=~ /(.*)(\.xml)/o){
               $filename = "../$1.xml.h";
    	       unlink($filename);
           }

           # Delete header files coming from glade files
           #--------------------------------------------
#           elsif ($_=~ /(.*)(\.glade)/o){
#               $filename = "../$1.glade.h";
#               unlink($filename);
#           }
       }
       close FILE;
    }
    print "done\n";
}

sub Merging{

    if ($ARGV[1]){
        $LANG   = $ARGV[1]; 
    } else {
	$LANG   = $ARGV[0];
    }

    if ($ARGV[0] ne "--dist" && $ARGV[0] ne "-D") {
        print "Merging $LANG.po with $PACKAGE.pot...";
    }

    $MERGE="cp $LANG.po $LANG.po.old && msgmerge $LANG.po.old $PACKAGE.pot -o $LANG.po";

    system($MERGE);
    
    if ($ARGV[0] ne "--dist" && $ARGV[0] ne "-D") {
        print "\n\n";
    }

    # Remove the "messages" trash file generated
    # by gettext, aswell as the backup file
    #-------------------------------------------
    unlink "messages";
    unlink "$LANG.po.old";
}

sub NotExisting{

    # Report error if supplied language 
    # file is non-existant
    #----------------------------------
    print "update.pl:  sorry, $LANG.po does not exist!\n";
    print "Try `update.pl --help' for more information.\n";    
    exit;
}

sub Status{

    # Print status information about the po file
    #-------------------------------------------
    $STATUS="msgfmt --statistics $LANG.po";
    
    system($STATUS);
    print "\n";   
}
