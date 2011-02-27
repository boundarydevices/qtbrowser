try {
   scanner.scanned.connect(function(barcode){ 
      window.location = "http://www.searchupc.com/default.aspx?q="+barcode;
   });
} catch (errmsg) {
   alert( "exception "+errmsg+" connecting to scanner");
   alert( "current location == " + window.location);
}

