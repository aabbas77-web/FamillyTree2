
><B
>HTTP_AUTH_DIGEST</B
></TT
> and/or <TT
CLASS="constant"
><B
>HTTP_AUTH_NTLM</B
></TT
></TD
></TR
><TR
><TD
>compress</TD
><TD
>bool</TD
><TD
>whether to allow gzip/deflate content encoding</TD
></TR
><TR
><TD
>port</TD
><TD
>int</TD
><TD
>use another port as specified in the url</TD
></TR
><TR
><TD
>referer</TD
><TD
>string</TD
><TD
>the referer to send</TD
></TR
><TR
><TD
>useragent</TD
><TD
>string</TD
><TD
>the user agent to send (defaults to PECL::HTTP/version (PHP/version))</TD
></TR
><TR
><TD
>headers</TD
><TD
>array</TD
><TD
>list of custom headers as associative array like array("header" =&#62; "value")</TD
></TR
><TR
><TD
>cookies</TD
><TD
>array</TD
><TD
>list of cookies as associative array like array("cookie" =&#62; "value")</TD
></TR
><TR
><TD
>encodecookies</TD
><TD
>bool</TD
><TD
>whether to urlencode the cookies (default: true)</TD
></TR
><TR
><TD
>cookiestore</TD
><TD
>string</TD
><TD
>path to a file where cookies are/will be stored</TD
></TR
><TR
><TD
>cookiesession</TD
><TD
>bool</TD
><TD
>don't load session cookies from cookiestore if TRUE</TD
></TR
><TR
><TD
>resume</TD
><TD
>int</TD
><TD
>byte offset to start the download from; if the server supports ranges</TD
></TR
><TR
><TD
>range</TD
><TD
>array</TD
><TD
>array of arrays, each containing two integers, specifying the ranges to download if server support is given; only recognized if the resume option is empty</TD
></TR
><TR
><TD
>maxfilesize</TD
><TD
>int</TD
><TD
>maximum file size that should be downloaded; has no effect, if the size of the requested entity is</TD
></TR
><TR
><TD
>lastmodified</TD
><TD
>int</TD
><TD
>timestamp for If-(Un)Modified-Since header</TD
></TR
><TR
><TD
>etag</TD
><TD
>string</TD
><TD
>quoted etag for If-(None-)Match header</TD
></TR
><TR
><TD
>timeout</TD
><TD
>int</TD
><TD
>seconds the request may take</TD
></TR
><TR
><TD
>connecttimeout</TD
><TD
>int</TD
><TD
>seconds the connect may take</TD
></TR
><TR
><TD
>onprogress</TD
><TD
>mixed</TD
><TD
>progress callback</TD
></TR
><TR
><TD
>interface</TD
><TD
>string</TD
><TD
>outgoing network interface (ifname, ip or hostname)</TD
></TR
><TR
><TD
>portrange</TD
><TD
>array</TD
><TD
>2 integers specifying outgoing portrange to try</TD
></TR
><TR
><TD
>ssl</TD
><TD
>array</TD
><TD
>&#13;        <P
>&#13;         <DIV
CLASS="table"
><A
NAME="http.request.options.ssl"
></A
><P
><B
>Table 2. with the following options:</B
></P
><TABLE
BORDER="1"
CLASS="CALSTABLE"
><COL><COL><COL><THEAD
><TR
><TD
>Name</TD
><TD
>Type</TD
><TD
>Description</TD
></TR
></THEAD
><TBODY
><TR
><TD
>cert</TD
><TD
>string</TD
><TD
>path to certificate</TD
></TR
><TR
><TD
>certtype</TD
><TD
>string</TD
><TD
>type of certificate</TD
></TR
><TR
><TD
>certpasswd</TD
><TD
>string</TD
><TD
>password for certificate</TD
></TR
><TR
><TD
>key</TD
><TD
>string</TD
><TD
>path to key</TD
></TR
><TR
><TD
>keytype</TD
><TD
>string</TD
><TD
>type of key</TD
></TR
><TR
><TD
>keypasswd</TD
><TD
>string</TD
><TD
>pasword for key</TD
></TR
><TR
><TD
>engine</TD
><TD
>string</TD