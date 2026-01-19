<?php

function lcg($seed,$m,$a,$c){
  $sequence=array();
  for($i=0;$i<$m;$i++){
    $seed=($a * $seed + $c) % $m;
    $sequence[]=$seed;
  }
  return $sequence;
}

$m=$argv[1];
$seq=lcg(rand(0,$m-1),$m,$argv[2],$argv[3]);
foreach($seq as $k => $v){
  file_put_contents("$m-$k.lcg","$v\n");
}
?>
