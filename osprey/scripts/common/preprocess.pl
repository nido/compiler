: # use perl
eval 'exec perl -S $0 ${1+"$@"}'
    if 0;
#
#
# usage: preprocess.pl files... -o output_file
#
# Concatenate and preprocess the given files.
# The preprocessing is just an expansion of every #include "..." directives.
#
# This script can be used for instance to create a single include file from a
# file containing include directives.
#

sub usage() {
  print STDOUT <<EOM
usage: $0 [options...] files.... >output_file

 Concatenate and preprocess the given files.
 The preprocessing consist in expanding \#include \"...\" directives
 and only these.

 This script can be used for instance to create a single include file from a
 file containing include directives.

 options are:
 -I <inc_path> : specify an additional include path
 -i : ignore missing include files
 -d : debug mode
 -h : this help screen

EOM
}

#use strict; # commented out to use file handles.

my @inc_path=();
my $debug = 0;
my @files = ();
my $ignore = 0;
my $usage = 0;
my $output;
my $opt_md = 0;
my $opt_mm = 0;
my $dep = 0;
my $depfile;

while(scalar(@ARGV)) {
  my $arg = $ARGV[0];
  if ($arg =~ /^-I(.*)$/) {
    if ($1 ne "") {
      push @inc_path, $1;
    } elsif (defined($ARGV[1])) {
      push @inc_path, $ARGV[1];
      shift @ARGV;
    } else {
      die "missing argumnet to -I option\n";
    }
    shift @ARGV;
  } elsif ($arg =~ /^-o(.*)$/) {
    if ($1 ne "") {
      $output = $1;
    } elsif (defined($ARGV[1])) {
      $output = $ARGV[1];
      shift @ARGV;
    } else {
      die "missing argumnet to -o option\n";
    }
    shift @ARGV;
  } elsif ($arg =~ /^-MF(.*)$/) {
    if ($1 ne "") {
      $depfile = $1;
    } elsif (defined($ARGV[1])) {
      $depfile = $ARGV[1];
      shift @ARGV;
    } else {
      die "missing argumnet to -MF option\n";
    }
    shift @ARGV;
  } elsif ($arg =~ /^--$/) {
    shift @ARGV;
    push @files, @ARGV;
    last;
  } elsif ($arg =~ /^-d$/) {
    $debug++;
    shift @ARGV;
  } elsif ($arg =~ /^-i$/) {
    $ignore++;
    shift @ARGV;
  } elsif ($arg =~ /^-MD$/) {
    $opt_md = 1;
    $dep = 1;
    shift @ARGV;
  } elsif ($arg =~ /^-MMD$/) {
    # same effect as -MD. system header are not processesd
    $opt_md = 1;
    $opt_mm = 1; # will be ignored
    $dep = 1;
    shift @ARGV;
  } elsif ($arg =~ /^-M$/ || $arg =~ /^-MM$/) {
    die "option not supported. Use -MD instead\n";
    shift @ARGV;
  } elsif ($arg =~ /^-.*/) {
    # ignore option
    shift @ARGV;
  } elsif ($arg =~ /^-h$/) {
    $usage++;
    shift @ARGV;  
  } else {
    shift @ARGV;
    push @files, $arg;
  }
}

sub basename {
  my ($file) = @_;
  my $basename = $file;
  $basename =~ s!^.*/+!!;
  return $basename;
}

sub basedir {
  my ($file) = @_;
  my $basename = basename($file);
  my $basedir = $file;
  $basedir =~ s!$basename!!;
  $basedir =~ s!/+$!!;
  $basedir = "." if $basedir eq "";
  return $basedir;
}

sub is_abs {
  my ($file) = @_;
  return $file =~ /^\//;
}

sub make_path {
  my ($dir, $file) = @_;
  my $full_path = "$dir/$file";
  $full_path =~ s!/+!/!g;
  return $full_path;
}

sub find_path {
  my ($dir, $file) = @_;
  return $file if is_abs($file);
  foreach my $path ($dir, @inc_path) {
    my $full_path = make_path($path,$file);
    return $full_path if -f $full_path;
  }
  return undef;
}

sub file_dir {
  my ($dir, $file) = @_;
  return basedir($file);
}
  
sub debug {
  my ($msg) = @_;
  print STDERR "$msg\n" if $debug > 0;
}

usage() if $usage;

die "missing file argument. Find help with -h option.\n" if scalar(@files) == 0;

die "dependency option -MD no compatible with stdout output.\n" if defined($dep) && !defined($output);

my %defines = ();
my @depfiles = ();
my $output_fh;
my $depfile_fh;
my $cwd = `/bin/pwd`;
my $gfh = "fh000000";

if (defined($output)) {
  $gfh++;
  open($gfh, ">$output") || die "cannot open $output for writing: $!\n";
  $output_fh = $gfh;
} else {
  $output_fh = \*STDOUT;
}

if ($debug>0) {
  debug("include path list is:");
  foreach my $path (@inc_path) {
    debug(" $path");
  }
}

my @stack;
foreach my $file (@files) {
  $gfh++;
  debug("opening $file in $gfh");
  open($gfh, "$file") || die "cannot open $file: $!\n";
  push @stack, [$file, $gfh, 0, file_dir($cwd, $file)];
  push @depfiles, $file;
  do {
    my $tuple = $stack[-1];
    my $fh;
    $fh = $$tuple[1];
    local $_;
    while(<$fh>) {
      $tuple->[2]++; # increment line number
      my ($fname, $fh, $fline, $fdir) = @$tuple;
      my $include_file;
      if (/^\s*\#\s*define\s+(\S+)\s+(.*)$/) {
	# record single line 
	debug("define $1 as: $2");
	$defines{$1} = $2;
	print $output_fh $_;
      }
      # Process #include <...>. Ignored
      elsif (/^\s*\#\s*include\s+\<(.*)\>/) {
	# ignored
	print $output_fh $_;
      }
      # Process #include "...". 
      elsif (/^\s*\#\s*include\s+\"(.*)\"/) {
	$include_file = $1;
      }
      # Process #include IDENT. Expand ident and retry
      elsif (/^\s*\#\s*include\s+(\w+)/) {
	$include_file = $defines{$1};
	die "$fname:$fline: undefined identifier in include directive: $1\n" if !defined($include_file);
	if ($include_file =~ /^\<.*\>$/) {
	  # ignore system include
	  $include_file = undef;
	  print $output_fh $_;
	} elsif ($include_file =~ /^\"(.*)\"$/) {
	  $include_file = $1;
	}
      }
      # other lines ar outputed as is
      else {
	print $output_fh $_;
      }
      if (defined($include_file)) {
	debug("finding file $include_file from dir $fdir");
	my $include_fname = find_path($fdir, $include_file);
	if (defined($include_fname)) {
	  $gfh++;
	  debug("opening $include_fname in $gfh");
	  open($gfh, "$include_fname") || die "cannot open $include_fname: $!\n";
	  push @stack, [$include_fname, $gfh, 0, file_dir($fdir, $include_fname)];
	  push @depfiles, $include_fname;
	  last;
	} else {
	  if ($ignore) {
	    debug("ignoring missing file: $include_basename");
	    print $output_fh $_;
	  } else {
	    die "$fname:$fline: file not found in include directive: $include_basename\n";
	  }
	}
      }
    }
    if ($tuple == $stack[-1]) {
      my ($fname, $fh) = @$tuple;
      debug("closing $fname");
     close($fh) || die "can't close fh for $fname: $!\n";
      pop @stack;
    }
  } while (scalar(@stack) > 0);
}

if ($output) {
  close($output_fh);
}

if (defined($dep) && defined($output)) {
  if (!defined($depfile)) {
    if (defined($opt_md)) {
      $depfile = basename($output);
      $depfile =~ s!.[^.]*$!!;
      $depfile = $depfile . ".d";
    } else {
      die "file output for dependency option not specified, use -MF option.\n";
    }
  }
  $gfh++;
  open($gfh, ">$depfile") || die "cannot open $depfile for writing: $!\n";
  $depfile_fh = $gfh;
  print $depfile_fh "$output: \\\n" if ($dep);
  foreach my $depfile (@depfiles) {
    print $depfile_fh " $depfile \\\n";
  }
  print $depfile_fh "\n";
  close($depfile_fh);
}

