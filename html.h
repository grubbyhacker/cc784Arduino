/*
 * Large string blobs for serving the root web page. The blobs are stored in PROGMEM to save runtime memory.
 * 
 * TODO: modify Esp8266WebServer to support PROGMEM streaming of content. Right now storing these strings in
 *       PROGMEM is a waste since the entire thing has to be pulled into memory anyway.
 */


/*
 * Many thanks to todbot.com (http://todbot.com/blog/2008/06/19/how-to-do-big-strings-in-arduino/)
 *
 */
void printProgStr(const prog_char str[], Print* p)
{
  char c;
  unsigned long bytesread = 0;
  if(!str) return;
  while((c = pgm_read_byte(str++))) {
    p->write(c);
    if (++bytesread % 1000 == 0) {
      yield(); // just to make sure the esp chip gets its time
    }
  }
}

/*
 *  This is the first part of the root web page served on /. Its is huge (around 6kb.) All whitespace has been removed except from
 *  the javascript section to save space. This chunk covers the entire file up to and including the table of metrics and headers.
 */
const char root_header[] PROGMEM = R"V0G0N(<!DOCTYPE html><!-- Template by html.am --><html><head> <meta content="text/html; charset=utf-8" http-equiv="Content-Type"> <title>ColorCells Model 784 Web Console</title> <style type="text/css"> body { margin:0; padding:0; font-family: Sans-Serif; line-height: 1.5em; } #header { background: #ccc; height: 100px; background-image: url(http://raspberrypi/cc784/cc784.jpg) } #header h1 { margin: 0; padding-top: 15px; color: MintCream; } main { padding-bottom: 10010px; margin-bottom: -10000px; float: left; width: 100%; } #nav { width: 180px; padding: 0 10px; right: 240px; margin-left: -100%; padding-bottom: 10010px; margin-bottom: -10000px; float: left; background: #eee; } iframe { position: fixed; top : 110px; right : 0; height : 60%; width : 30%; padding: 10px 10px; } #footer { clear: left; width: 100%; background: #ccc; text-align: center; padding: 4px 0; } .resultsmessage { position: fixed; top : 110px; right : 0; height : 60%; width : 30%; padding: 10px 10px; font-family:Geneva, sans-serif; font-size:2em; letter-spacing:0.2em; line-height:100px; text-align:center; color:#FFFFFF; background-color:#000000; } #content { margin-left: 230px; /* Same as 'nav' width */margin-right: 35%; } .innertube { margin: 15px; /* Padding for content */ margin-top: 0; } p { color: #555; } nav ul { list-style-type: none; margin: 0; padding: 0; } nav ul a { color: darkgreen; text-decoration: none; } textarea { width: 60%; } .myTable { width: 400px; background-color: #eee; border-collapse: collapse; } .myTable th { background-color: #000; color: white; width: 50%; } .myTable td, .myTable th { padding:5px; border:1px solid #000; } </style><script type="text/javascript">

    var doloading = function(){
        var l = document.getElementById("loadingMessage");
        l.style.visibility='visible';
        var i = document.getElementById("result");
        i.style.visibility='hidden';
    }

    var doloaded = function(){
      var l = document.getElementById("loadingMessage");
      l.style.visibility='hidden';
      var i = document.getElementById("result");
      i.style.visibility='visible';
    }

    var showinfomessage = function(){
    var html_string= "<!DOCTYPE html><html><head><style type=\"text/css\" scoped>" +
             ".GeneratedText {font-family:Geneva, sans-serif;font-size:2em;" +
             "letter-spacing:0.2em;line-height:1.3em;text-align:left;" +
             "color:#FFFFFF;background-color:#000000;padding:1.5em;}<\/style>" +
                 "<\/head><body><div class=\"GeneratedText\">" +
             "The results of commands sent to the CC784 will be sent here.<\/div>" +
             "<\/body><\/html>";
    document.getElementById('result').src = "data:text/html;charset=utf-8," + escape(html_string);
    }

    </script>
</head>
<body onload="showinfomessage();"> <header id="header"> <div class="innertube"> <h1>Color Cells 784</h1> </div> </header> <iframe id="result" name="result" onload= "doloaded();"></iframe> <div class="resultsmessage" id="loadingMessage"> Command is running.... </div> <div id="wrapper"> <main> <div id="content"> <div class="innertube"> <h1>WIFI Enabled CC784</h1> <p></p> <h3 id="message">Change Message</h3> <p></p>Messages follow a simple protocol: <ul><li>Every string of characters must be preceeded by "s:"</li><li>Every color cells command must be preceeded by "c:"</li><li>Sequences of strings and commands must be separated by a comma.</li></ul>For example: "s:Hellow World." will give you a simple string of yellow font. But"s:Hello ,c:BOLD,s:World,c:NORMAL,s:." will give you the same string only with "World" in bold. <form action="/program" method="get" target="result"><br> Select a message bank: <select name="bank"> <option value="0"> 0 </option> <option value="1"> 1 </option> <option value="2"> 2 </option> <option value="3"> 3 </option> <option value="4"> 4 </option> <option value="5"> 5 </option> <option value="6"> 6 </option> <option value="7"> 7 </option> <option value="8"> 8 </option> <option value="9"> 9 </option> </select><br> <textarea maxlength="300" name="msg">Enter text here...</textarea><br> <input onclick="doloading();" type="submit" value= "Submit"> </form> <h3 id="sequence">Sequence Messages</h3>There are 10 banks of messages, numbered 0-9. You can schedule which banks of messages will be displayed in any order. Repeats are allowed. For instance: <ul><li>01234 will sequence the first 5 banks.</li><li>99994 will repeat bank 9 four times, then play bank 4.</li></ul>There is a limit of 32 items in the sequence. <p></p> <form action="/sequence" method="get" target="result">Input your sequence: <input type="text" name="seq"><input onclick="doloading();" type="submit" value= "Submit"> </form> <h3 id="metrics">Serving Metrics</h3> <table class="myTable"> <tr> <th>Metric</th> <th>Value</th> </tr>)V0G0N";

/*
 *  This chunk closes out the table and the rest of the file.
 *
 */
const char root_footer[] PROGMEM = R"V0G0N(</table> </div> </div> </main> <nav id="nav"> <div class="innertube"> <h3>Commands</h3> <ul> <li> <a href="/stop" onclick="doloading();" target="result">Stop</a> </li> <li> <a href="/run" onclick="doloading();" target="result">Run</a> </li> <li> <a href="/rebootdisplay" onclick="doloading();" target= "result">Reboot Sign</a> </li> <li> <a href="/signoff" onclick="doloading();" target= "result">Turn Sign Off (NYI)</a> </li> <li> <a href="/signon" onclick="doloading();" target= "result">Turn Sign On (NYI)</a> </li> <li> <a href="#message">Change Message</a> </li> <li> <a href="#sequence">Sequence</a> </li> </ul> <h3>Information</h3> <ul> <li> <a href="#metrics">Serving Metrics</a> </li> <li> <a href="" target="_blank">Protocol Definition</a> </li> <li> <a href="" target="_blank">CC784 Manual</a> </li> </ul> </div> </nav> </div> <footer id="footer"> <div class="innertube"> <p></p> </div> </footer></body></html>)V0G0N";

