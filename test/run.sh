#!/bin/sh

dir=`dirname $0`
$dir/../tibia $dir/product.json,$dir/company.json,$dir/vst3.json $dir/../templates/vst3 $dir/../out/vst3
$dir/../tibia $dir/product.json,$dir/company.json,$dir/vst3.json,$dir/vst3-make.json $dir/../templates/vst3-make $dir/../out/vst3
cp $dir/plugin.h $dir/../out/vst3/src

$dir/../tibia $dir/product.json,$dir/company.json,$dir/lv2.json $dir/../templates/lv2 $dir/../out/lv2
$dir/../tibia $dir/product.json,$dir/company.json,$dir/lv2.json $dir/../templates/lv2-make $dir/../out/lv2
cp $dir/plugin.h $dir/../out/lv2/src

$dir/../tibia $dir/product.json,$dir/company.json $dir/../templates/web $dir/../out/web
$dir/../tibia $dir/product.json,$dir/company.json $dir/../templates/web-make $dir/../out/web
$dir/../tibia $dir/product.json,$dir/company.json $dir/../templates/web-demo $dir/../out/web
cp $dir/plugin.h $dir/../out/web/src

$dir/../tibia $dir/product.json,$dir/company.json,$dir/android.json $dir/../templates/android $dir/../out/android
$dir/../tibia $dir/product.json,$dir/company.json,$dir/android.json,$dir/android-make.json $dir/../templates/android-make $dir/../out/android
cp $dir/keystore.jks $dir/../out/android
cp $dir/plugin.h $dir/../out/android/src
