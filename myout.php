<?php

include "data.php";
$istat=$argv[1];
$query=oci_parse($conn,"select sovra from idistat where istat='$istat'");
oci_execute($query);
$row=oci_fetch_row($query);
@$sovra=$row[0];
oci_free_statement($query);
$comune="  \"comune\":{\n";
$unione="  \"unione\":{\n";
$regione="  \"regione\":{\n";

function show1($table,$par,$title,$istat,$sovra,$conn){
  global $comune,$regione;
  $query=oci_parse($conn,"select $par from $table where istat='$istat'");
  oci_execute($query);
  $row=oci_fetch_row($query);
  @$aux=(int)$row[0];
  oci_free_statement($query);
  $comune.="    \"$table\":$aux,\n";
  $query=oci_parse($conn,"select $par from $table where istat='$sovra'");
  oci_execute($query);
  $row=oci_fetch_row($query);
  @$aux=(int)$row[0];
  oci_free_statement($query);
  $unione.="    \"table\":$aux,\n";
  $query=oci_parse($conn,"select $par from $table where istat='00008'");
  oci_execute($query);
  $row=oci_fetch_row($query);
  @$aux=(int)$row[0];
  oci_free_statement($query);
  $regione.="    \"table\":$aux,\n";
}

$conn=oci_connect($p1,$p2,$p3);

show1("uiftth","uiftth","FTTH bianche",$istat,$sovra,$conn);
show1("areeaai","areeaai","AAI Aree",$istat,$sovra,$conn);
show1("aziendeaai","aziendeaai","AAI Aziende",$istat,$sovra,$conn);
show1("scuole","scuole","Scuole 1G",$istat,$sovra,$conn);
show1("pal","pal","PAL rete",$istat,$sovra,$conn);
show1("man","man","MAN rete",$istat,$sovra,$conn);
show1("apwifi","apwifi","Punti WiFi",$istat,$sovra,$conn);
show1("userwifi","userwifi","Utenti WiFi",$istat,$sovra,$conn);
show1("attivifse","attivi","Attivi FSE",$istat,$sovra,$conn);
show1("accessifse","accessi","Accessi FSE",$istat,$sovra,$conn);
show1("scaricatifse","scaricati","Scaricati FSE",$istat,$sovra,$conn);
show1("attivazionilepidaid","attivazioni","Attivazioni ID",$istat,$sovra,$conn);
show1("accessilepidaid","accessi","Accessi ID",$istat,$sovra,$conn);
show1("sportellilepidaid","sportelli","Sportelli ID",$istat,$sovra,$conn);

$comune.="    \"istat\":\"$istat\"\n  }\n";
$unione.="    \"istat\":\"$sovra\"\n  }\n";
$regione.="    \"istat\":\"00008\"\n  }\n";
echo "{\n";
echo $comune;
echo $unione;
echo $regione;
echo "}\n";

oci_close($conn);

?>
