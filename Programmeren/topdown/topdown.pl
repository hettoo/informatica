#!/usr/bin/perl

our $PROGRAM = 'topdown';
our $VERSION = '0.4';

use strict;
use warnings;
use feature qw(switch say);

use autodie;
use Getopt::Long;

use Tie::IxHash;
use XML::Simple;

########
# Global constants
########

# noisy strings
my $EMPTY   = q{};
my $SPACE   = q{ };
my $NEWLINE = qq{\n};
my $DOT     = q{.};
my $COMMA   = q{,};
my $DQUOTE  = q{"};
my $SLASH   = q{/};

# directories
my $CD = $DOT . $SLASH;
my $PD = $DOT . $DOT . $SLASH;

# config file and locations
my $CONFIG_FILE         = 'topdown.xml';
my @STATIC_CONFIG_FILES = (
    $CD . $CONFIG_FILE,
    $ENV{ $^O =~ /Win/ ? 'USERPROFILE' : 'HOME' } 
        . $SLASH 
        . $DOT
        . $CONFIG_FILE,
    '/etc/' . $CONFIG_FILE,
);

# regexes (regices?) for the parser
tie my %REGEX, 'Tie::IxHash',
    (
    'linec'           => qr{//},
    'lblockc'         => qr{/[*]},
    'rblockc'         => qr{[*]/},
    'macro'           => qr/#/,
    'esquote'         => qr/\\'/,
    'squote'          => qr/'/,
    'edquote'         => qr/\\"/,
    'dquote'          => qr/"/,
    'lparen'          => qr/[(]/,
    'comma'           => qr/,/,
    'rparen'          => qr/[)]/,
    'lbrace'          => qr/{/,
    'rbrace'          => qr/}/,
    'lt'              => qr/</,
    'gt'              => qr/>/,
    'pointref'        => qr/[*&]/,
    'semicolon'       => qr/;/,
    'word'            => qr/\w+/,
    'escaped_newline' => qr/\\\R/,
    'newline'         => qr/\R/,
    'whitespace'      => qr/\s+/,
    'other'           => qr/./,
    );

# the alphabet, used to translate numbers to letters
my @ALPHABET = 'a' .. 'z';

########
# Global variables
########

# user options
my $config;
my $filetype;
my $remove;
my $application;

# other global variables
my $settings;
my %ignore;
my %total_ignore;
my %only;
my %type_generating_keyword;
my %keyword;
my %type;
my %proto;
my %info;
my %defined;
my %used;
my %calls;
my %drawn;

########
# Utilities
########

sub load_file {
    my ($file) = @_;
    my $text;
    local $/ = undef;
    open my $in, '<', $file;
    $text = <$in>;
    close $in;
    return $text;
}

sub real_file {
    my ($file) = @_;
    $file =~ s{\Q$CD\E}{}g;
    $file =~ s{\Q$PD\E([^/])/}{$1}g;
    return $file;
}

sub get_dir {
    my ($file) = @_;
    my $dir = $file;
    $dir = substr $dir, 0, 1 + rindex $dir, $SLASH;
    return $dir;
}

########
# Parser
########

sub parse {
    my ($file) = @_;
    $file = real_file($file);
    my $state = init_parser($file);
    tie %{ $state->{'calls'}{ $state->{'function'}->[0] } }, 'Tie::IxHash';
    my $stop;
    do {
        $stop = 1;
        for my $type ( keys %REGEX ) {
            $state->{'type'} = $type;
            my $regex = $REGEX{ $state->{'type'} };
            if ( $state->{'text'} =~ s/^($regex)// ) {
                $state->{'result'} = $1;
                $stop = 0;
                if ( !pre_parse($state) ) {
                    last;
                }
                &{ \&{ 'parse_' . $state->{'type'} } }($state);
                post_parse($state);
                last;
            }
        }
    } while ( !$stop );
}

sub init_parser {
    my ($file) = @_;
    return {
        'line'          => [1],
        'ignore_line'   => 0,
        'blockc'        => 0,
        'squote'        => 0,
        'dquote'        => 0,
        'function'      => [ $settings->{'pre_main'} ],
        'declaring'     => 0,
        'calling'       => [],
        'arity'         => [],
        'ignore_parens' => [],
        'braces'        => 0,
        'macro'         => 0,
        'macro_type'    => $EMPTY,
        'include'       => $EMPTY,
        'included'      => { $file => 1 },
        'file'          => [$file],
        'bytes_left'    => [undef],
        'dir'           => get_dir($file),
        'clean'         => 1,
        'text'          => load_file($file),
    };
}

sub update_line_number {
    my ($state) = @_;
    if ( $state->{'type'} eq 'newline' ) {
        $state->{'line'}[-1]++;
    }
    elsif ( $state->{'type'} eq 'whitespace' ) {
        $state->{'line'}[-1] += $state->{'result'} =~ /\R/g;
    }
}

sub update_bytes_left {
    my ($state) = @_;
    if ( defined $state->{'bytes_left'}[-1] ) {
        $state->{'bytes_left'}[-1] -= length $state->{'result'};
        if ( $state->{'bytes_left'}[-1] == 0 ) {
            pop @{ $state->{'line'} };
            pop @{ $state->{'file'} };
            pop @{ $state->{'bytes_left'} };
            $state->{'dir'} = get_dir( $state->{'file'}[-1] );
        }
    }
}

sub ignore {
    my ($state) = @_;
    if (   ( $state->{'ignore_line'} && $state->{'type'} ne 'newline' )
        || ( $state->{'blockc'} && $state->{'type'} ne 'rblockc' )
        || ( $state->{'squote'} && $state->{'type'} ne 'squote' )
        || ( $state->{'dquote'} && $state->{'type'} ne 'dquote' ) )
    {
        if (   $state->{'dquote'}
            && $state->{'macro_type'} eq 'include' )
        {
            $state->{'include'} .= $state->{'result'};
        }
        return 1;
    }
    return 0;
}

sub update_args {
    my ($state) = @_;
    if ( ( @{ $state->{'calling'} } || $state->{'declaring'} )
        && $state->{'type'} ne 'rparen' )
    {
        $state->{'args'} .= $state->{'result'};
    }
}

sub pre_parse {
    my ($state) = @_;
    update_line_number($state);
    update_bytes_left($state);
    if ( ignore($state) ) {
        return 0;
    }
    update_args($state);
    return 1;
}

sub parse_macro {
    my ($state) = @_;
    $state->{'macro'} = 1;
}

sub parse_linec {
    my ($state) = @_;
    $state->{'ignore_line'} = 1;
    $state->{'prev_clean'}  = $state->{'clean'};
}

sub parse_lblockc {
    my ($state) = @_;
    $state->{'blockc'} = 1;
}

sub parse_rblockc {
    my ($state) = @_;
    $state->{'blockc'} = 0;
}

sub parse_squote {
    my ($state) = @_;
    $state->{'squote'} = !$state->{'squote'};
}

sub parse_dquote {
    my ($state) = @_;
    $state->{'dquote'} = !$state->{'dquote'};
}

sub parse_lparen {
    my ($state) = @_;
    if ( @{ $state->{'words'} } && !$total_ignore{ $state->{'words'}[-1] } ) {
        if ( $state->{'function'}->[0] eq $settings->{'pre_main'}
            && @{ $state->{'words'} } >= 2 )
        {
            parse_lparen_declaring($state);
            push @{ $state->{'ignore_parens'} }, 0;
        }
        else {
            if ( parse_lparen_calling($state) ) {
                push @{ $state->{'ignore_parens'} }, 0;
            }
        }
    }
    if (@{ $state->{'ignore_parens'} }
        && ((   @{ $state->{'words'} }
                && $total_ignore{ $state->{'words'}[-1] }
            )
            || !@{ $state->{'words'} }
        )
        )
    {
        ${ $state->{'ignore_parens'} }[-1]++;
    }
    @{ $state->{'words'} } = ();
}

sub parse_lparen_declaring {
    my ($state) = @_;
    $state->{'candidate_function'} = $state->{'words'}[-1];
    $state->{'declaring'}          = 1;
    $type{ $state->{'words'}[-1] } = $state->{'words'}[-2];
    $type{ $state->{'words'}[-1] } =~ s/</&amp;lt;/g;
    $type{ $state->{'words'}[-1] } =~ s/>/&amp;gt;/g;
}

sub parse_lparen_calling {
    my ($state) = @_;
    for my $test ( 0 .. $#{ $state->{'words'} } - 1 ) {
        if ( $state->{'words'}[$test] =~ /^\w+$/
            && !$keyword{ $state->{'words'}[$test] } )
        {
            return 0;
        }
    }
    if ( $state->{'words'}[-1] =~ /\W$/ ) {
        @{ $state->{'words'} } = ();
        return 0;
    }
    push @{ $state->{'calling'} }, $state->{'words'}[-1];
    push @{ $state->{'arity'} }, ( $state->{'text'} =~ /^\s*[)]/ ? 0 : 1 );
    return 1;
}

sub parse_rparen {
    my ($state) = @_;
    if (   @{ $state->{'ignore_parens'} }
        && ${ $state->{'ignore_parens'} }[-1] )
    {
        ${ $state->{'ignore_parens'} }[-1]--;
    }
    else {
        if ( @{ $state->{'calling'} } ) {
            parse_rparen_calling($state);
        }
        elsif ( $state->{'declaring'} ) {
            parse_rparen_declaring($state);
        }
        undef $state->{'args'};
    }
}

sub word {
    my ($n) = @_;
    my $word = '';
    do {
        $word = $ALPHABET[ $n % @ALPHABET ];
        $n    = int $n / @ALPHABET;
    } while ( $n > 0 );
    return $word;
}

sub apply_arity {
    my ( $function, $arity ) = @_;
    for ( my $i = 0; $defined{ $function . '(' . $arity . ')' }; $i++ ) {
        $arity =~ s/[^\d]+$//;
        $arity .= word($i);
    }
    my $new_function = $function . '(' . $arity . ')';
    $arity =~ s/\D//g;
    $type{$new_function} = $type{$function};
    @{ $proto{$new_function} } = @{ $proto{$function} }[ 0 .. $arity - 1 ];
    $info{$new_function} = $info{$function};
    return $new_function;
}

sub add_arity {
    my ($function) = @_;
    my @arities;
    for my $i ( 0 .. $#{ $proto{$function} } ) {
        if ( $proto{$function}->[$i] =~ s/=.*// ) {
            push @arities, $i;
        }
    }
    push @arities, scalar @{ $proto{$function} };
    my @new_functions;
    for my $arity (@arities) {
        push @new_functions, apply_arity( $function, $arity );
    }
    return @new_functions;
}

sub get_alternatives {
    my ( $function, $arity ) = @_;
    my @alt;
    for ( my $i = 0; $defined{ $function . '(' . $arity . ')' }; $i++ ) {
        push @alt, $arity;
        $arity =~ s/[^\d]+$//;
        $arity .= word($i);
    }
    return @alt;
}

sub show_alternatives {
    my ( $state, $call, @alt ) = @_;
    say '';
    say 'Call to function ' 
        . $call . ' at '
        . $state->{'file'}[-1]
        . ' line '
        . $state->{'line'}[-1]
        . ' is ambiguous (for now). '
        . 'Please choose the corresponding function prototype:';
    for my $i ( 0 .. $#alt ) {
        say "$i $info{$call.'('.$alt[$i].')'}";
    }
}

sub choose_alternative {
    my (@alt) = @_;
    my $choice;
    do {
        print '>>> ';
        $choice = <STDIN>;
        chomp $choice;
    } while ( $choice < 0 || $choice > $#alt );
    return $choice;
}

sub register_call {
    my ( $state, $call ) = @_;
    for my $i ( 0 .. $#{ $state->{'function'} } ) {
        if ( $state->{'function'}->[$i] ne $call ) {
            $used{$call} = 1;
        }
        if ( $state->{'function'}->[$i] eq $settings->{'pre_main'} ) {
            $defined{ $state->{'function'}->[$i] } = 1;
            $type{ $state->{'function'}->[$i] }    = 'void';
        }
        $calls{ $state->{'function'}->[$i] }{$call}++;
    }
}

sub parse_rparen_calling {
    my ($state) = @_;
    my $call    = pop @{ $state->{'calling'} };
    my $arity   = pop @{ $state->{'arity'} };
    my @alt = get_alternatives( $call, $arity );
    if ( @alt <= 1 ) {
        $call .= '(' . $arity . ')';
    }
    else {
        show_alternatives( $state, $call, @alt );
        my $choice = choose_alternative(@alt);
        $call .= '(' . $alt[$choice] . ')';
    }
    if ( $state->{'function'}->[0] ne $EMPTY ) {
        register_call( $state, $call );
    }
}

sub get_args {
    my ($state) = @_;
    my @args;
    if ( defined $state->{'args'} ) {
        @args = split /,/, $state->{'args'};
    }
    for my $arg (@args) {
        $arg =~ s/\s+/ /g;
        $arg =~ s/^ //;
        $arg =~ s/ $//;
    }
    return @args;
}

sub args_name_only {
    my (@args) = @_;
    for my $arg (@args) {
        my $is_array = $arg =~ s/\[.*\]//;
        $arg =~ s/=.*/=/;
        $arg =~ s/\s*([<>])\s*/$1/g;
        while ( $arg !~ /^\W*[\w<>]+\s*=?$/ && $arg =~ s/^\s*[\w<>]+// ) { }
        $arg =~ s/.*<.*>//;
        $arg =~ s/\s//g;
        if ($is_array) {
            $arg = '*' . $arg;
        }
    }
    return @args;
}

sub parse_rparen_declaring {
    my ($state) = @_;
    $state->{'declaring'} = 0;
    my @args = get_args($state);
    $info{ $state->{'candidate_function'} }
        = '('
        . $state->{'file'}[-1]
        . ' line '
        . $state->{'line'}[-1] . '): '
        . join ', ', @args;
    @args = args_name_only(@args);
    @{ $proto{ $state->{'candidate_function'} } } = @args;
}

sub parse_comma {
    my ($state) = @_;
    if ( @{ $state->{'calling'} } ) {
        $state->{'arity'}[-1]++;
    }
}

sub set_candidate_as_function {
    my ($state) = @_;
    @{ $state->{'function'} } = add_arity( $state->{'candidate_function'} );
    for my $i ( 0 .. $#{ $state->{'function'} } ) {
        if ( !$calls{ $state->{'function'}->[$i] } ) {
            tie %{ $calls{ $state->{'function'}->[$i] } }, 'Tie::IxHash';
        }
        $defined{ $state->{'function'}->[$i] } = 1;
    }
    undef $state->{'candidate_function'};
}

sub parse_lbrace {
    my ($state) = @_;
    if (   $state->{'clean'}
        && @{ $state->{'words'} } >= 2
        && $type_generating_keyword{ $state->{'words'}[-2] } )
    {
        $total_ignore{ $state->{'words'}[-1] } = 1;
    }
    elsif ( defined $state->{'candidate_function'} ) {
        set_candidate_as_function($state);
    }
    @{ $state->{'words'} } = ();
    $state->{'braces'}++;
}

sub parse_rbrace {
    my ($state) = @_;
    $state->{'braces'}--;
    if ( $state->{'braces'} == 0 ) {
        @{ $state->{'function'} } = ( $settings->{'pre_main'} );
    }
    $state->{'clean'} = 1;
}

sub parse_semicolon {
    my ($state) = @_;
    if ( defined $state->{'candidate_function'} ) {
        add_arity( $state->{'candidate_function'} );
        undef $state->{'candidate_function'};
    }
    $state->{'clean'} = 1;
}

sub parse_pointref {
    my ($state) = @_;
    if ( @{ $state->{'words'} } ) {
        $state->{'words'}[-1] .= $state->{'result'};
    }
}

sub parse_word {
    my ($state) = @_;
    if ( $state->{'macro'} ) {
        $state->{'macro_type'} = $state->{'result'};
        if ( $state->{'macro_type'} ne 'include' ) {
            $state->{'ignore_line'} = 1;
            $state->{'prev_clean'}  = $state->{'clean'};
        }
    }
    if ( $state->{'result'} !~ /^\d+$/ ) {
        if (   @{ $state->{'words'} }
            && $state->{'words'}->[-1] =~ /</
            && $state->{'words'}->[-1] !~ />/ )
        {
            $state->{'words'}->[-1] .= $state->{'result'};
        }
        else {
            push @{ $state->{'words'} }, $state->{'result'};
        }
    }
}

sub apply_include {
    my ($state) = @_;
    my $new_file = real_file( $state->{'dir'} . $state->{'include'} );
    if ( !$state->{'included'}{$new_file} ) {
        my $included = load_file($new_file);
        push @{ $state->{'line'} },       1;
        push @{ $state->{'file'} },       $new_file;
        push @{ $state->{'bytes_left'} }, 1 + length $included;
        $state->{'dir'}  = get_dir( $state->{'file'}[-1] );
        $state->{'text'} = $included . $NEWLINE . $state->{'text'};
        $state->{'included'}{$new_file} = 1;
    }
}

sub parse_newline {
    my ($state) = @_;
    if ( $state->{'include'} ne $EMPTY ) {
        apply_include($state);
    }
    if ( $state->{'ignore_line'} ) {
        $state->{'clean'}       = $state->{'prev_clean'};
        $state->{'ignore_line'} = 0;
    }
    if ( $state->{'macro'} ) {
        @{ $state->{'words'} } = ();
        $state->{'macro'}      = 0;
        $state->{'macro_type'} = $EMPTY;
        $state->{'include'}    = $EMPTY;
    }
    $state->{'whitespace'} = 1;
}

sub parse_whitespace {
    my ($state) = @_;
    $state->{'whitespace'} = 1;
}

sub parse_other {
    my ($state) = @_;
    if ( !$state->{'declaring'} ) {
        undef $state->{'candidate_function'};
    }
}

sub parse_lt {
    my ($state) = @_;
    if ( @{ $state->{'words'} }
        && $state->{'words'}->[-1] =~ /^\s*vector\s*(?:<\s*vector\s*)*$/ )
    {
        $state->{'words'}->[-1] .= $state->{'result'};
    }
}

sub parse_gt {
    my ($state) = @_;
    if ( @{ $state->{'words'} } && $state->{'words'}->[-1] =~ /</ ) {
        $state->{'words'}->[-1] .= $state->{'result'};
    }
}

sub post_parse {
    my ($state) = @_;
    if ( $state->{'type'} ne 'newline' && $state->{'type'} ne 'whitespace' ) {
        if (   $state->{'type'} ne 'word'
            && $state->{'type'} ne 'pointref'
            && (   !$state->{'words'}
                || !@{ $state->{'words'} }
                || (!(     $state->{'type'} eq 'lt'
                        && $state->{'words'}->[-1] =~ /<$/
                    )
                    && !(
                           $state->{'type'} eq 'gt'
                        && $state->{'words'}->[-1] =~ />/
                    )
                )
            )
            )
        {
            @{ $state->{'words'} } = ();
            if (   $state->{'type'} ne 'rbrace'
                && $state->{'type'} ne 'semicolon'
                && $state->{'type'} ne 'lblockc'
                && $state->{'type'} ne 'rblockc' )
            {
                $state->{'clean'} = 0;
            }
        }
        $state->{'whitespace'} = 0;
    }
}

########
# Processor
########

sub merge_setting(\%$) {
    my ( $attr, $name ) = @_;
    @{$attr}{ keys %{ $settings->{$name} } } = values %{ $settings->{$name} };
}

sub raw {
    my ($function) = @_;
    $function =~ s/\(\w+\)$//;
    return $function;
}

sub function_tree {
    my ( $function, @above ) = @_;
    my $result = $EMPTY;
    my $id = get_id( $function, @above );
    if ( $proto{$function} && @{ $proto{$function} } ) {
        $result .= function_arguments( $function, $id );
    }
    elsif ( !@above ) {
        $result .= create_dummy($id);
    }
    my %attr;
    if ( !@above ) {
        merge_setting( %attr, 'top_node' );
    }
    elsif ( !$defined{$function} ) {
        merge_setting( %attr, 'undefined_node' );
    }
    if ( !$calls{$function} || !%{ $calls{$function} } ) {
        if ( $type{$function} ) {
            if ( $defined{$function} ) {
                merge_setting( %attr, 'bottom_node' );
            }
        }
        else {
            merge_setting( %attr, 'undeclared_node' );
        }
    }
    elsif ($drawn{$function}
        && !$settings->{'draw_duplicate_trees'}
        && keys %{ $calls{$function} } > 0 )
    {
        merge_setting( %attr, 'duplicate_node' );
    }
    else {
        $result .= function_calls( $function, @above );
    }
    $attr{'label'} = (
        $function =~ /[(]/
        ? substr $function,
        0,
        index $function,
        '('
        : $function
    );
    $result .= node( $id, \%attr );
    $drawn{$function} = 1;
    return $result;
}

sub type_label {
    my ( $function, $label ) = @_;
    my $return_type_label = $settings->{'return_type_label'};
    $return_type_label =~ s/%l/$type{$function}/;
    return $return_type_label
        . ( defined $label ? '<br/>' . $label : $EMPTY );
}

sub function_calls {
    my ( $function, @above ) = @_;
    my $result = $EMPTY;
    my $id = get_id( $function, @above );
    for my $call ( keys %{ $calls{$function} } ) {
        my %edge_attr;
        if ( $calls{$function}{$call} > 1 ) {
            $edge_attr{'label'} = '(' . $calls{$function}{$call} . 'x)';
        }
        my $void = !$type{$call} || $type{$call} eq 'void';
        if ($void) {
            merge_setting( %edge_attr, 'void_edge' );
        }
        else {
            $edge_attr{'label'} = type_label( $call, $edge_attr{'label'} );
        }
        my @new_above;
        my $recursive = 0;
        for my $above ( @above, $function ) {
            if ( $above eq $call ) {
                $recursive = 1;
                last;
            }
            push @new_above, $above;
        }
        my $call_id = get_id( $call, @new_above );
        if ($recursive) {
            if ( !$void ) {
                merge_setting( %edge_attr, 'non_void_recursive_edge' );
            }
            $result .= edge( $call_id, $id, \%edge_attr );
        }
        else {
            if ( !$void ) {
                merge_setting( %edge_attr, 'non_void_edge' );
            }
            $result .= edge( $id, $call_id, \%edge_attr );
            $result .= function_tree( $call, @new_above );
        }
    }
    return $result;
}

sub function_arguments {
    my ( $function, $id ) = @_;
    my $result = $EMPTY;
    for my $arg ( @{ $proto{$function} } ) {
        my $arg_id = $id . '-' . $arg;
        $result .= node( $arg_id, $settings->{'empty_node'} );
        my %arg_attr;
        if ( $arg =~ /[*&]/ ) {
            merge_setting( %arg_attr, 'reference_arg_edge' );
        }
        else {
            merge_setting( %arg_attr, 'normal_arg_edge' );
        }
        my $new_arg = $arg;
        $new_arg =~ s/[^<>\w]//g;
        $new_arg =~ s/</&lt;/g;
        $new_arg =~ s/>/&gt;/g;
        $arg_attr{'label'} = $new_arg;
        $result .= edge( $arg_id, $id, \%arg_attr );
    }
    return $result;
}

sub create_dummy {
    my ($id)     = @_;
    my $result   = $EMPTY;
    my $dummy_id = $id . '-dummy';
    $result .= node( $dummy_id, $settings->{'dummy_node'} );
    $result .= edge( $dummy_id, $id, $settings->{'dummy_edge'} );
    return $result;
}

sub get_id {
    my ( $function, @above ) = @_;
    return join '>', @above, $function;
}

sub graph_init {
    my ( $type, $name, $attr ) = @_;
    my $result = $type . 'graph ' . $name . '{' . $NEWLINE;
    for my $key ( keys %{$attr} ) {
        if ( ( ref $attr->{$key} ) eq 'HASH' ) {
            $result .= $key . '[' . get_attr( \%{ $attr->{$key} } ) . ']';
        }
        else {
            $result .= get_attr( { $key => $attr->{$key} } );
        }
        $result .= $NEWLINE;
    }
    return $result;
}

sub node {
    my ( $name, $attr ) = @_;
    my $result = $EMPTY;
    $result
        .= $DQUOTE . $name . $DQUOTE . '[' . get_attr($attr) . ']' . $NEWLINE;
    return $result;
}

sub edge {
    my ( $top, $down, $attr ) = @_;
    my $result = $EMPTY;
    $result
        .= $DQUOTE 
        . $top 
        . $DQUOTE . '->' 
        . $DQUOTE 
        . $down 
        . $DQUOTE . '['
        . get_attr($attr) . ']'
        . $NEWLINE;
    return $result;
}

sub graph_finish {
    return '}' . $NEWLINE;
}

sub get_attr {
    my ($attr) = @_;
    my @items;
    for my $key ( keys %{$attr} ) {
        $attr->{$key} =~ s/\s*\R+\s*//g;
        push @items, $key . '=<' . $attr->{$key} . '>';
    }
    return join $COMMA, @items;
}

########
# Standard program output
########

sub help {
    print <<"EOF";
NAME
    $PROGRAM - generate top-down schemes from C++ files

SYNOPSIS
    $0 FILES [OPTIONS]

OPTIONS
    -a, --application=APPLICATION
        View the generated output file using APPLICATION. Implies -f.

    -c, --config=CONFIG
        Manually specify a config file.

    -f, --filetype[=FILETYPE]
        Run the graphviz post-processor specified in the config file, generating
        output in the specified filetype, which is assumed to be the default
        filetype specified in the config file when omitted.

    -h, --help
        Display this help and exit.

    -r, --remove
        Remove the generated graphviz file. Implies -f.

    -v, --version
        Output version information and exit.
EOF
    exit;
}

sub version {
    say "$PROGRAM version $VERSION";
    exit;
}

########
# Main subroutines
########

sub load_options {
    Getopt::Long::Configure('bundling');
    GetOptions(
        'h|help'          => \&help,
        'v|version'       => \&version,
        'c|config=s'      => \$config,
        'f|filetype:s'    => \$filetype,
        'r|remove'        => \$remove,
        'a|application=s' => \$application,
    );
    if ( ( $remove || defined $application ) && !defined $filetype ) {
        $filetype = $EMPTY;
    }
}

sub get_config_file {
    if ( defined $config ) {
        if ( !-e $config ) {
            die 'config file ' . $config . ' does not exist';
        }
    }
    else {
        my @input_config_files = @ARGV;
        for my $config_file (@input_config_files) {
            $config_file .= '.' . $CONFIG_FILE;
        }
        for my $config_file ( @input_config_files, @STATIC_CONFIG_FILES ) {
            if ( -e $config_file ) {
                $config = $config_file;
                last;
            }
        }
        if ( !defined $config ) {
            die 'no config file found';
        }
    }
}

sub load_settings {
    get_config_file();
    $settings = XMLin( $config, SuppressEmpty => $EMPTY );
    %ignore = map { $_ => 1 } split /\s+/, $settings->{'ignore'};
    %type_generating_keyword = map { $_ => 1 } split /\s+/,
        $settings->{'type_generating_keywords'};
    %keyword = map { $_ => 1 } split /\s+/, $settings->{'keywords'};
    @ignore{ keys %keyword } = values %keyword;
    %total_ignore = %ignore;
    %only = map { $_ => 1 } split /\s+/, $settings->{'draw_only'};

    if ( defined $filetype && $filetype eq $EMPTY ) {
        $filetype = $settings->{'default_filetype'};
    }
}

sub reset_global_variables {
    %total_ignore = %ignore;
    undef %type;
    undef %proto;
    undef %info;
    undef %defined;
    undef %used;
    undef %calls;
    undef %drawn;
}

sub remove_undeclared {
    for my $function ( keys %calls ) {
        for my $call ( keys %{ $calls{$function} } ) {
            if ( !$defined{$call} && !$type{$call} ) {
                delete $calls{$function}{$call};
            }
        }
    }
}

sub create_graphs {
    for my $file (@ARGV) {
        parse($file);
        if ( !$settings->{'draw_undeclared_nodes'} ) {
            remove_undeclared();
        }
        my $graph = graph_init( 'di', 'scheme', $settings->{'graph'} );
        for my $function ( keys %type ) {
            if (   $defined{$function}
                && !$used{$function}
                && ( !keys %only || $only{ raw($function) } ) )
            {
                $graph .= function_tree($function);
            }
        }
        $graph .= graph_finish();
        post_process( $file, $graph );
        reset_global_variables();
    }
}

sub post_process {
    my ( $file, $graph ) = @_;
    my $graph_file = $file . $settings->{'extension'};
    open my $out, '>', $graph_file;
    print $out $graph;
    close $out;
    if ( defined $filetype ) {
        system $settings->{'post_processor'}
            . " -T$filetype "
            . ( quotemeta $graph_file ) . '>'
            . ( quotemeta $file . $DOT . $filetype );
        if ($remove) {
            unlink $graph_file;
        }
        if ( defined $application ) {
            system $application . $SPACE
                . ( quotemeta $file . $DOT . $filetype );
        }
    }
}

# main program
load_options();
load_settings();
create_graphs();
