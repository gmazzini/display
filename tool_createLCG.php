<?php

// https://dspace.library.uvic.ca/bitstream/handle/1828/3142/Random_Number_Generators.pdf?sequence=3

include "data.php";
$conn=oci_connect($p1,$p2,$p3);

// $qq="create table rnd1 (p integer,m integer,num integer,primary key (p))";
// $qq="create table rnd2 (m integer,id integer,a integer,c integer,primary key (m,id))";

$query=oci_parse($conn,"delete from rnd1");
oci_execute($query);
oci_free_statement($query);
$query=oci_parse($conn,"delete from rnd2");
oci_execute($query);
oci_free_statement($query);

$m1=1000;
$m2=2*$m1;
for($i=2;$i<=$m2;$i++)$f[$i]=1;
for($i=2;$i<=$m2;$i++)if($f[$i])for($j=$i+$i;$j<=$m2;$j+=$i)$f[$j]=0;

for($m1=2;$m1<=1000;$m1++){
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
        
        $query=oci_parse($conn,"insert into rnd2 (m,id,a,c) values ($m,$cc,$a,$c)");
        oci_execute($query);
        oci_free_statement($query);
        $cc++;
      }
    }
    if($cc>0)break;
  }
}
oci_close($conn);

function gcd($a,$b){
  while($b!=0){
    $t=$b;
    $b=$a % $b;
    $a=$t;
  }
  return $a;
}

?>
