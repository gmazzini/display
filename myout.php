<?php

include "data.php";
$istat=$argv[1];
$sovra="";
$comune="  \"comune\":{\n";

function show1($table,$par,$title,$istat,$sovra,$conn){
  global $comune;
  $query=oci_parse($conn,"select $par from $table where istat='$istat'");
  oci_execute($query);
  $row=oci_fetch_row($query);
  @$aux=(int)$row[0];
  oci_free_statement($query);
  $comune.="    \"$par\":$aux,\n";
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
echo "{\n";
echo $comune;
echo "}\n";

oci_close($conn);

?>
