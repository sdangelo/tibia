#!/bin/sh

dir=`dirname $0`
$dir/../tibia $dir/product.json,$dir/company.json,$dir/vst3.json $dir/../templates/vst3 $dir/../out
