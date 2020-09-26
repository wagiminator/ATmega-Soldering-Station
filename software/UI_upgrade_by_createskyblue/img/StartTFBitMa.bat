@echo off
for %%s in (ÉèÖÃ\*.png) do (
echo %%s
java -jar compressor2.0.jar %%s -anp %%s >>bitmaps.h
)
pause