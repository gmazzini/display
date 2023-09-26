<?php

include "data.php";

$conn=oci_connect($p1,$p2,$p3);

for($i=42;$i<=63;$i++){
  for($j=0;$j<=255;$j++){
    $id=$i*256+$j;
    $query=oci_parse($conn,"select istat from idistat where $id>=idstart and $id<=idend");
    oci_execute($query);
    $row=oci_fetch_row($query);
    if(isset($row[0]))$myistat[$id]=$row[0];
    else $myistat[$id]=0;
    oci_free_statement($query);
  }
}

$i=0;
$ii=0;
$sock=socket_create(AF_INET,SOCK_DGRAM,0);
socket_bind($sock, "0.0.0.0",514);
for(;;){
  socket_recvfrom($sock,$buf,1000,0,$remote_ip,$remote_port);
  $ll=strpos($buf,":");
  $ll=strpos($buf,":",$ll+1);
  $ll=strpos($buf,":",$ll+1);
  $aux=substr($buf,$ll+2);
  if(substr($aux,0,11)=="DHCPREQUEST"){
    $ll=strpos($buf,"for");
    $le=strpos($buf," ",$ll+4);
    $ip=substr($buf,$ll+4,$le-$ll-4);
    $ll=strpos($buf,"from");
    $le=strpos($buf," ",$ll+5);
    $mac=substr($buf,$ll+5,$le-$ll-5);
    $aa=explode(".",$ip);
    if($aa[0]=="10" && $aa[1]>=32 && $aa[1]<=63){
      $id=$aa[1]*256+$aa[2];
      if($myistat[$id]==0)echo "Missed:$ip\n";
      else {
        $i++;
        $vv=substr(hash("sha256",$mac),0,8);

        $query=oci_parse($conn,"select count(*) from logwifi where id='$vv' and ip=$id");
        oci_execute($query);
        $row=oci_fetch_row($query);
        @$myexist=$row[0];
        oci_free_statement($query);

        if(!$myexist){
          $istat=$myistat[$id];
          $tt=time();
          $stmt=oci_parse($conn,"insert into logwifi values ('$vv',$id,$tt,'$istat')");
          oci_execute($stmt);
           $ii++;
           echo "$i,$ii,$ip,$id,$vv,$istat,$tt\n";
        }
      }
    }
  }
}
?>
