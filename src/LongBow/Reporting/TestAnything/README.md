
# Generating the About

This is a little bit of a chicken and egg.  You need to have installed LongBow
already so you have /usr/bin/longbow-generate-about and the assocaited Python
modules installed.  Once that is done, you can then execute:

```
longbow-generate-about longBowReportTestAnything TestAnything \"0.1\" mini-notice short-notice long-notice 
```

where the files `mini-notice`, `short-notice`, and `long-notice` need to
exist with your desired messages in the current directory.


