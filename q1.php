<?php

// https://dspace.library.uvic.ca/bitstream/handle/1828/3142/Random_Number_Generators.pdf?sequence=3

$m1=$argv[1];
$m2=10*$m1;

for($i=2;$i<=$m2;$i++)$f[$i]=1;
for($i=2;$i<=$m2;$i++)if($f[$i])for($j=$i+$i;$j<=$m2;$j+=$i)$f[$j]=0;


for($m=$m1;$m<$m2;$m++){
  unset($q);
  for($i=2;$i<=$m;$i++)$q[$i]=($f[$i])?(($m%$i==0)?1:0):0;
  $cc=0;
  for($a=2;$a<$m;$a++){
    for($c=1;$c<$m;$c++){
      if(gcd($m,$c) != 1)continue;
      for($p=2;$p<=$m;$p++){
        if(!$q[$p])continue;
        if($a % $p != 1)break;
      }
      if($p<=$m)continue;
      if($m % 4 ==0 && $a % 4 != 1)continue;
      $oo[$cc]="$m,$a,$c";
      $cc++;
    }
  }
  if($cc>0)break;
}

print_r($oo);

function gcd($a,$b){
  while($b!=0){
    $t=$b;
    $b=$a % $b;
    $a=$t;
  }
  return $a;
}

?>
