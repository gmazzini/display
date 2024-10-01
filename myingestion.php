<?php

include "data.php";
$conn=oci_connect($p1,$p2,$p3);
$mode=$argv[1]; // 0 from UDP/514, 1 from STDIN

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
$uttt=0;
if($mode==0){
  $sock=socket_create(AF_INET,SOCK_DGRAM,0);
  socket_bind($sock, "0.0.0.0",514);
}
else if($mode==1){
  $mfp=fopen("php://stdin","r");
}

for(;;){
  if($mode==0){
    socket_recvfrom($sock,$buf,1000,0,$remote_ip,$remote_port);
  }
  else if($mode==1){
    $aux=fgets($mfp);
    if($aux===false){
      fclose($mfp);
      break;
    }
    $buf=trim($aux);
  }
  
  $ll=strpos($buf," ");
  $ll=strpos($buf," ",$ll+1);
  $ll=strpos($buf," ",$ll+1);
  $aux=substr($buf,0,$ll);
  $ttt=strtotime($aux);
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
        $xx=explode(":",$mac);
        $tohash="";
        for($j=0;$j<6;$j++)$tohash.=chr(hexdec($xx[$j]));
        $vv2=hash("fnv1a64",$tohash,false);
        $tt=(int)($ttt/86400);
        $istat=$myistat[$id];

        $query=oci_parse($conn,"select req from dhcpwifi where fnv1a='$vv2' and ip=$id and tt=$tt");
        oci_execute($query);
        $row=oci_fetch_row($query);
        @$myreq=(int)$row[0];
        oci_free_statement($query);

        if($myreq==0){
          $query=oci_parse($conn,"insert into dhcpwifi (fnv1a,ip,tt,istat,req) values (hextoraw('$vv2'),$id,$tt,'$istat',1)");
          oci_execute($query);
          oci_free_statement($query);
          $ii++;
          if($ttt>$uttt){
            echo "N,$i,$ii,$ip,$id,$vv2,$istat,$tt,1\n";
            $uttt=$ttt+2;
          }
        }
        else {
          $myreq++;
          $query=oci_parse($conn,"update dhcpwifi set req=$myreq where fnv1a='$vv2' and ip=$id and tt=$tt");
          oci_execute($query);
          oci_free_statement($query);
          if($ttt>$uttt){
            echo "U,$i,$ii,$ip,$id,$vv2,$istat,$tt,$myreq\n";
            $uttt=$ttt+2;
          } 
        }
      }
    }
  }
}
?>
