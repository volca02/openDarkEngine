@REM $Id$
@REM Requires GraphWiz DOT and Texi2html to be present and in the path

@Echo OFF
IF "%1"=="" GOTO die

mkdir %1\img

for %%d in (*.dot) do dot -T png -o%1\img\%%~nd.png %%d
texi2html.pl -output %1\ -split=section -top_file=index.html -init_file opdetexi2html.init manual.texi -P %1\
Exit /B 0

:die
Echo No destination path specified
Exit /B 1