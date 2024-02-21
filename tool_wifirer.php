<?php

include "data.php";
$conn=oci_connect($p1,$p2,$p3);
$tt=(int)$argv[1];
$lab=array("11271"=>"Presidente","11452"=>"AM38","11456"=>"AM5052","11460"=>"AM18","11464"=>"AM21","11468"=>"AM30","11472"=>"AM44","11476"=>"AM64","11480"=>"Fiera8","11484"=>"Silvani","11500"=>"Galliera","11502"=>"Marsala","14438"=>"StacpPR","15112"=>"POP","15231"=>"ImolaARL","15296"=>"Campus");

$query=oci_parse($conn,"select idstart,idend from idistat where eistat='00008'");
oci_execute($query);
for($cc=0;;$cc++){
  $row=oci_fetch_row($query);
  if($row==null)break;
  $idstart[$cc]=(int)$row[0];
  $idend[$cc]=(int)$row[1];
}
oci_free_statement($query);

for($c=0;$c<$cc;$c++){
  for($n=0;$n<50;$n+=5){
    $query=oci_parse($conn,"select count(*) from dhcpwifi where ip>=$idstart[$c] and ip<=$idend[$c] and tt=$tt and req>$n");
     oci_execute($query);
     $row=oci_fetch_row($query);
     $nn[$c][$n]=(int)$row[0];
     oci_free_statement($query);
  }
}

for($c=0;$c<$cc;$c++){
  printf("%d,%s",$tt,$lab[$idstart[$c]]);
  for($n=0;$n<50;$n+=5)printf(",%d",$nn[$c][$n];
  printf("\n");          
}

oci_close($conn);
?>
