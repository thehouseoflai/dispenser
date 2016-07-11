#!/usr/bin/perl

use Net::MQTT::Simple;
use App::Daemon 'daemonize';

my $id = '16299908';
my $tag = '070075D961';
my $mqtt = Net::MQTT::Simple->new('localhost');

# TODO: wrap all this in a cron loop of some kind
# TODO: get the ID, tag, and timing from a database

daemonize();

$mqtt->publish("dispenser/$id", 'rfid_enable');
$mqtt->run("dispenser/$id/received", sub {
  my ($topic, $message) = @_;
  warn "[$topic] $message\n";
  if ($message eq $tag) {
    $mqtt->publish("dispenser/$id", 'servo_run');
    $mqtt->publish("dispenser/$id", 'rfid_disable');
    exit(0);
  }
});

