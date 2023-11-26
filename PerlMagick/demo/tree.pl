#!/usr/bin/perl
#
# Example of using a lsys fractal,
# which in turm used Turtle Graphics
#
use Cwd;
use lib cwd;
require "lsys.pl";

%rule = (
	  'A' => 'S[---LMA][++++B]',
          'B' => 'S[++LBg][--Cg]',
          'C' => 'S[-----LB]GS[+MC]',
          'g' => '',
          'L' => '[{S+S+S+S+S+S}]'
        );

%stemchanges = (
  distance => 18.5,
  dtheta => 0.1,
  motionsub => sub{
                    $im->Draw ( primitive=>'line', points=>join(' ',@_),
                                stroke=>'dark green', strokewidth=>1 );
                  }
);

%polychanges = (
  distance => 3,
  dtheta => 0.4,
  motionsub => sub{ push( @poly, @_[0..1] ); }
);

$changes = \%stemchanges;
lsys_init(400);
lsys_execute('A', 10, "tree.pam", %rule);
