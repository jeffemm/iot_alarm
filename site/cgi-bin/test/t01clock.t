#!/opt/local/bin/perl -w

use Test::Simple tests => 47;

use JSON;
use Log::Log4perl qw( get_logger :levels);
use ArduinoClock;

Log::Log4perl->init( "perl_logger.conf" );

my $t_clk = ArduinoClock->new;
ok( defined($t_clk),                'new() object constructor' );

ok( 0 == $t_clk->hour(0),           'hour() set accessor, valid value 0' );
ok( 0 == $t_clk->minute(0),         'minute() set accessor, valid value 0' );
ok( 0 == $t_clk->second(0),         'second() set accessor, valid value 0' );
ok( 0 == $t_clk->day(0),            'day() set accessor, valid value 0' );

ok( 5 == $t_clk->hour(5),           'hour() set accessor, valid value' );
ok( 6 == $t_clk->minute(6),         'minute() set accessor, valid value' );
ok( 7 == $t_clk->second(7),         'second() set accessor, valid value' );
ok( 3 == $t_clk->day(3),            'day() set accessor, valid value' );

ok( !defined( $t_clk->hour(35) ),   'hour() set accessor, out of range value' );
ok( !defined( $t_clk->minute(73) ), 'minute() set accessor, out of range value' );
ok( !defined( $t_clk->second(80) ), 'second() set accessor, out of range value' );
ok( !defined( $t_clk->day(9) ),     'day() set accessor, out of range value' );

ok( 5 == $t_clk->hour,              'hour() get accessor' );
ok( 6 == $t_clk->minute,            'minute() get accessor' );
ok( 7 == $t_clk->second,            'second() get accessor' );
ok( 3 == $t_clk->day,               'day() get accessor' );

ok( '{"Clock":"0506073"}' eq $t_clk->get_ar_json_str,                   'get_ar_json_string' );

my $s = $t_clk->get_ua_json_str;
my $j = decode_json $s;
ok( 5 == $j->{'clock'}->{'hr'},                             'get_ua_json_str(), hour' );
ok( 6 == $j->{'clock'}->{'mn'},                             'get_ua_json_str(), minute' );
ok( 7 == $j->{'clock'}->{'sc'},                             'get_ua_json_str(), second' );
ok( 3 == $j->{'clock'}->{'dy'},                             'get_ua_json_str(), day' );

$s = '{"clock":{"hr":"8","mn":"9","sc":"10","dy":"2"}}';
ok( defined( $t_clk->set_ua_json_str( $s ) ),               'set_ua_json_str(), valid input' );
ok( 8 == $t_clk->hour,                                      'set_ua_json_str(), hour correct' );
ok( 9 == $t_clk->minute,                                    'set_ua_json_str(), minute correct' );
ok( 10 == $t_clk->second,                                   'set_ua_json_str(), second correct' );
ok( 2 == $t_clk->day,                                       'set_ua_json_str(), day correct' );
ok( !defined( $t_clk->set_ua_json_str( "ckahkeaha" ) ),     'set_ua_json_str(), garbage input' );

ok( defined( $t_clk->set_ar_json_str( '{"Clock":"1112131"}' ) ),      'set_ar_json_str(), valid input' );
ok( 11 == $t_clk->hour,                                                 'hour correct after set_ar_json_str' );
ok( 12 == $t_clk->minute,                                               'minute correct after set_ar_json_str' );
ok( 13 == $t_clk->second,                                               'second correct after set_ar_json_str' );
ok( 1 == $t_clk->day,                                                   'day correct after set_ar_json_str' );

ok( !defined( $t_clk->set_ar_json_str( '{"Clock":"3112131"}' ) ),     'set_ar_json_str(), out of range hour' );
ok( !defined( $t_clk->set_ar_json_str( '{"Clock":"1182131"}' ) ),     'set_ar_json_str(), out of range minute' );
ok( !defined( $t_clk->set_ar_json_str( '{"Clock":"1112931"}' ) ),     'set_ar_json_str(), out of range second' );
ok( !defined( $t_clk->set_ar_json_str( '{"Clock":"1112139"}' ) ),     'set_ar_json_str(), out of range day' );
ok( !defined( $t_clk->set_ar_json_str( '{"Clxck":"1112139"}' ) ),     'set_ar_json_str(), invalid element name' );
ok( !defined( $t_clk->set_ar_json_str( '"Clxck":"1112139"}' ) ),     'set_ar_json_str(), invalid json string' );

ok( 11 == $t_clk->hour,                                                 'hour not modified by invalid set_ar_json_str' );
ok( 12 == $t_clk->minute,                                               'minute not modified by invalid set_ar_json_str' );
ok( 13 == $t_clk->second,                                               'second not modified by invalid set_ar_json_str' );
ok( 1 == $t_clk->day,                                                   'day not modified by invalid set_ar_json_str' );

$s = '{"Clock":"0000000"}';
ok( defined( $t_clk->set_ar_json_str( $s ) ),                         'set_ar_json_str(), call, test value ' . $s );
ok( $s eq $t_clk->get_ar_json_str,                                      'set_ar_json_str(), check, test value ' . $s );
$s = '{"Clock":"2359596"}';
ok( defined( $t_clk->set_ar_json_str( $s ) ),                         'set_ar_json_str(), call, test value ' . $s );
ok( $s eq $t_clk->get_ar_json_str,                                      'set_ar_json_str(), check, test value ' . $s );


