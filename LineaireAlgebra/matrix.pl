#!/usr/bin/perl

use strict;
use warnings;
use feature 'say';

use Getopt::Long;

my $INDENT = ' ' x 4;

my $invert;
my $variables;

main();

sub main {
    my $matrix = [];
    my $augment = [];
    get_options();
    get_input($matrix, $augment);
    solve($matrix, $augment);
}

sub get_options {
    GetOptions(
        'help' => \&help,
        'invert' => \$invert,
        'variables=s' => \$variables
    );
}

sub help {
    say "Usage: $0 [OPTIONS] FILES";
    say '';
    say 'OPTIONS:';
    say '    -h|--help       show this screen and exit';
    say '    -i|--invert     invert the matrix';
    say '    -v|--variables  set the variable names so that they can be solved';
    say '';
    say 'VARIABLE NAMES:';
    say '    a.  a, b, c, ...';
    say '    .z  ..., x, y, z';
    say '    x+  x1, x2, x3, ...';
    exit;
}

sub get_input {
    my($matrix, $augment) = @_;
    while (my $line = <>) {
        chomp $line;
        my @segments = split /\|/, $line;
        for my $segment (@segments) {
            $segment =~ s/^\s*(.*?)\s*$/$1/;
        }
        if (@segments >= 1) {
            push @{$matrix}, [split /\s+/, $segments[0]];
            if (@segments >= 2) {
                push @{$augment}, [split /\s+/, $segments[1]];
            }
        }
    }
    for my $row(@{$matrix}, @{$augment}) {
        for my $element(@{$row}) {
            if ($element =~ m%(\d+)/(\d+)%) {
                $element = [$1, $2];
            } else {
                $element = [$element, 1];
            }
        }
    }
}

sub solve {
    my($matrix, $augment) = @_;
    if (defined $invert) {
        show_matrix($matrix);
        identity_augment($matrix, $augment);
    } else {
        zero_augment($matrix, $augment);
    }
    make_echelon($matrix, $augment);
    if (defined $invert) {
        make_identity($matrix, $augment);
    } elsif (defined $variables) {
        show_solution($matrix, $augment);
    }
}

sub identity_augment {
    my($matrix, $augment) = @_;
    for my $i (0 .. $#{$matrix}) {
        my $width = @{$matrix->[$i]};
        @{$augment->[$i]} = ();
        for (my $j = 0; $j < $i; $j++) {
            push @{$augment->[$i]}, [0, 1];
        }
        push @{$augment->[$i]}, [1, 1];
        for (my $j = 0; $j < $width - $i - 1; $j++) {
            push @{$augment->[$i]}, [0, 1];
        }
    }
}

sub zero_augment {
    my($matrix, $augment) = @_;
    for my $i (0 .. $#{$augment}) {
        if (!@{$augment->[$i]}) {
            $augment->[$i] = [[0, 1]];
        }
    }
}

sub make_echelon {
    my($matrix, $augment) = @_;
    show_matrix($matrix, $augment);
    while (echelon_step($matrix, $augment)) {
        show_matrix($matrix, $augment);
    }
}

sub echelon_step {
    my($matrix, $augment) = @_;
    COLUMN: for my $i (0 .. $#{$matrix->[0]}) {
        my $first_zero;
        my $first_pivot;
        for my $j (0 .. $#{$matrix}) {
            if ($matrix->[$j]->[$i]->[0] == 0) {
                if (!defined $first_zero) {
                    $first_zero = $j;
                }
            } else {
                if (defined $first_pivot) {
                    my $quotient = [];
                    @{$quotient} = @{$matrix->[$j]->[$i]};
                    divide_value($quotient, $matrix->[$first_pivot]->[$i]);
                    $quotient->[0] *= -1;
                    add($matrix, $augment, $j, $first_pivot, $quotient);
                    return 1;
                }
                if (is_pivot($matrix, $j, $i)) {
                    $first_pivot = $j;
                }
                if (defined $first_zero && defined $first_pivot
                        && $first_pivot == $j && $j != $i) {
                    switch($matrix, $augment, $j, $first_zero);
                    return 1;
                }
            }
        }
    }
    return 0;
}

sub is_echelon {
    my($matrix) = @_;
    return pivots($matrix) == @{$matrix->[0]};
}

sub make_identity {
    my($matrix, $augment) = @_;
    if (!is_echelon($matrix)) {
        return;
    }
    for (my $i = $#{$matrix->[0]}; $i > 0; $i--) {
        for (my $j = $i - 1; $j >= 0; $j--) {
            if ($matrix->[$j]->[$i]->[0] != 0) {
                my $quotient = [];
                @{$quotient} = @{$matrix->[$j]->[$i]};
                divide_value($quotient, $matrix->[$i]->[$i]);
                $quotient->[0] *= -1;
                add($matrix, $augment, $j, $i, $quotient);
                show_matrix($matrix, $augment);
            }
        }
    }
    for (my $i = 0; $i < @{$matrix->[0]}; $i++) {
        if ($matrix->[$i]->[$i]->[0] != $matrix->[$i]->[$i]->[1]) {
            my $quotient = [];
            @{$quotient} = @{$matrix->[$i]->[$i]};
            invert_value($quotient);
            multiply($matrix, $augment, $i, $quotient);
            show_matrix($matrix, $augment);
        }
    }
    show_matrix($augment);
}

sub show_solution {
    my($matrix, $augment) = @_;
    if (!is_echelon($matrix)) {
        return;
    }
    my $width = @{$matrix->[0]};
    my @variables;
    my @indices = 1 .. $width;
    if ($variables =~ /(\w)\+/) {
        @variables = map {$1 . '_' . $_} @indices;
    } elsif ($variables =~ /(\w)\./) {
        @variables = map {chr $_ - 1 + ord $1} @indices;
    } elsif ($variables =~ /\.(\w)/) {
        @variables = map {chr $_ - $width + ord $1} @indices;
    } else {
        return;
    }
    say '\begin{align*}';
    my @values;
    for (my $i = $width - 1; $i >= 0; $i--) {
        print $INDENT . $variables[$i] . ' &= '
            . ($i == $width - 1 ? '' : '(') . format_value($augment->[$i]->[0]);
        $values[$i] = $augment->[$i]->[0];
        for (my $j = $width - 1; $j > $i; $j--) {
            my $subtract = [];
            @{$subtract} = @{$matrix->[$i]->[$j]};
            multiply_value($subtract, $values[$j]);
            subtract_value($values[$i], $subtract);
            print ' - ' . format_value($matrix->[$i]->[$j])
                . ' \cdot ' . format_value($values[$j]);
        }
        if ($i != $width - 1) {
            print ')';
        }
        my $inverted = [];
        @{$inverted} = @{$matrix->[$i]->[$i]};
        invert_value($inverted);
        multiply_value($values[$i], $inverted);
        say ' \cdot ' . format_value($inverted) . ' &= '
            . format_value($values[$i]) . ($i == 0 ? '' : ' \\\\');
    }
    say '\end{align*}';
}

sub pivots {
    my($matrix) = @_;
    my $pivots = 0;
    for my $i (0 .. $#{$matrix}) {
        for my $j (0 .. $#{$matrix->[$i]}) {
            $pivots += is_pivot($matrix, $i, $j);
        }
    }
    return $pivots;
}

sub is_pivot {
    my($matrix, $row, $column) = @_;
    if ($matrix->[$row]->[$column]->[0] == 0) {
        return 0;
    }
    for my $i (0 .. $column - 1) {
        if ($matrix->[$row]->[$i]->[0] != 0) {
            return 0;
        }
    }
    return 1;
}

sub multiply {
    my($matrix, $augment, $row, $times) = @_;
    for my $element (@{$matrix->[$row]}) {
        multiply_value($element, $times);
    }
    for my $element (@{$augment->[$row]}) {
        multiply_value($element, $times);
    }
    show_action('R_' . ($row + 1) . ' := ' . format_value($times)
        . 'R_' . ($row + 1));
}

sub add {
    my($matrix, $augment, $row, $source, $times) = @_;
    for my $i (0 .. $#{$matrix->[$row]}) {
        my $add = [];
        @{$add} = @{$matrix->[$source]->[$i]};
        multiply_value($add, $times);
        add_value($matrix->[$row]->[$i], $add);
    }
    for my $i (0 .. $#{$augment->[$row]}) {
        my $add = [];
        @{$add} = @{$augment->[$source]->[$i]};
        multiply_value($add, $times);
        add_value($augment->[$row]->[$i], $add);
    }
    my $subtract = 0;
    if (($times->[0] < 0) ^ ($times->[1] < 0)) {
        $subtract = 1;
        multiply_value($times, [-1, 1]);
    }
    my $ignore_quotient = 0;
    if ($times->[0] == $times->[1]) {
        $ignore_quotient = 1;
    }
    show_action('R_' . ($row + 1) . ' := R_' . ($row + 1) . ' '
        . ($subtract ? '-' : '+') . ' '
        . ($ignore_quotient ? '' : format_value($times))
        . 'R_' . ($source + 1));
}

sub switch {
    my($matrix, $augment, $a, $b) = @_;
    my @temp = ($matrix->[$a], $augment->[$a]);
    ($matrix->[$a], $augment->[$a]) = ($matrix->[$b], $augment->[$b]);
    ($matrix->[$b], $augment->[$b]) = @temp;
    show_action('R_' . ($a + 1) . ' \leftrightarrow R_' . ($b + 1));
}

sub show_matrix {
    my($matrix, $augment) = @_;
    my $augment_width = 0;
    if (defined $augment && !just_zero($augment)) {
        $augment_width = @{$augment->[0]};
    }
    my $width = @{$matrix->[0]};
    say '\[';
    say $INDENT . '\begin{pmatrix}['
        . ('c' x $width) . ($augment_width == 0 ? '' : '|'
            . ('c' x $augment_width)) . ']';
    for my $i (0 .. $#{$matrix}) {
        show_row($matrix->[$i], $augment_width == 0 ? undef : $augment->[$i]);
        say $i == $#{$matrix} ? '' : ' \\\\';
    }
    say $INDENT . '\end{pmatrix}';
    say '\]';
}

sub show_row {
    my($row, $augment) = @_;
    print $INDENT x 2;
    for my $i (0 .. $#{$row}) {
        print format_value($row->[$i])
            . ($i == $#{$row} && !defined $augment ? '' : ' & ');
    }
    if (defined $augment) {
        for my $i (0 .. $#{$augment}) {
            print format_value($augment->[$i])
                . ($i == $#{$augment} ? '' : ' & ');
        }
    }
}

sub show_action {
    my($action) = @_;
    say '$' . $action . '$';
}

sub just_zero {
    my($matrix) = @_;
    for my $row (@{$matrix}) {
        for my $element (@{$row}) {
            if ($element->[0] != 0) {
                return 0;
            }
        }
    }
    return 1;
}

sub add_value {
    my($a, $b) = @_;
    my $add_top = $a->[1] * $b->[0];
    $a->[0] *= $b->[1];
    $a->[1] *= $b->[1];
    $a->[0] += $add_top;
}

sub subtract_value {
    my($a, $b) = @_;
    add_value($a, [-$b->[0], $b->[1]]);
}

sub multiply_value {
    my($a, $b) = @_;
    $a->[0] *= $b->[0];
    $a->[1] *= $b->[1];
}

sub divide_value {
    my($a, $b) = @_;
    $a->[0] *= $b->[1];
    $a->[1] *= $b->[0];
}

sub invert_value {
    my($a) = @_;
    my $temp = $a->[0];
    $a->[0] = $a->[1];
    $a->[1] = $temp;
}

sub format_value {
    my($value) = @_;
    if ($value->[0] < 0 && $value->[1] < 0) {
        @{$value} = (abs $value->[0], abs $value->[1]);
    }
    if ($value->[0] == 0) {
        return 0;
    }
    if ($value->[1] == 0) {
        return '\infty';
    }
    my $div = $value->[0] / $value->[1];
    if ($div == int $div) {
        return $div;
    }
    my $gcd = gcd((abs $value->[0]), abs $value->[1]);
    return '\frac{' . $value->[0] / $gcd . '}{' . $value->[1] / $gcd . '}';
}

sub gcd {
    my($a, $b) = @_;
    if ($a == 0) {
        return $b;
    }
    if ($a > $b) {
        return gcd($b, $a);
    }
    return gcd($a, $b % $a);
}
