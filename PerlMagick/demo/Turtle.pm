package
  Turtle;

# Written by jreed@itis.com, adapted by Cristy.

sub new
{
  my $class = shift;
  my $self = {};

  @{$self}{qw(x y theta mirror)} = @_;
  bless $self, $class;
}

sub forward
{
  my $self = shift;
  my ($r, $what) = @_;
  my ($newx, $newy)=($self->{x}+$r* sin($self->{theta}),
                     $self->{y}+$r*-cos($self->{theta}));
  if ($what) {
    &$what($self->{x}, $self->{y}, $newx, $newy);  # motion
  }
  # According to the coderef passed in
  ($self->{x}, $self->{y})=($newx, $newy);  # change the old coords
}

sub turn
{
  my $self = shift;
  my $dtheta = shift;

  $self->{theta} += $dtheta*$self->{mirror};
}

sub state
{
  my $self = shift;

  @{$self}{qw(x y theta mirror)};
}

sub setstate
{
  my $self = shift;

  @{$self}{qw(x y theta mirror)} = @_;
}

sub mirror
{
  my $self = shift;

  $self->{mirror} *= -1;
}

"Turtle.pm";
