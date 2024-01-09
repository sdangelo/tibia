#!/bin/sh

dir=`dirname $0`
$dir/../tibia $dir/product.json,$dir/company.json,$dir/vst3.json $dir/../templates/vst3 $dir/../out/vst3
$dir/../tibia $dir/product.json,$dir/company.json,$dir/vst3.json,$dir/make.json,$dir/vst3-make.json $dir/../templates/vst3-make $dir/../out/vst3
cp $dir/plugin.h $dir/../out/vst3/src
$dir/../tibia $dir/product.json,$dir/company.json,$dir/lv2.json $dir/../templates/lv2 $dir/../out/lv2
$dir/../tibia $dir/product.json,$dir/company.json,$dir/lv2.json,$dir/make.json $dir/../templates/lv2-make $dir/../out/lv2
cp $dir/plugin.h $dir/../out/lv2/src
