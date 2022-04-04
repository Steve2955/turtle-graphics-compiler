
" Kleines Testprogramm: "Schnecke" aus Quadraten
"
" Optional ein Befehlszeilen-Wert: Skalierungsfaktor
"
" Klaus Kusche, 2022

path sqr(len)
  do 4 times
    walk testfunc(len)
    turn left 90
  done
endpath

" nur um eine Funktion zu testen
calculation testfunc(a)
  mul a by 1.1
returns a
endcalc

begin
  if @1 <> 0
  then store @1 in scale
  else store 1 in scale
  endif

  counter i from 0 to 360 step 10 do
    mark
    direction i
    sub 3 from @green
    add 3 to @blue
    jump scale
    path sqr(scale * (2 + i  / 48))
    div @delay by 1.2
    jump mark
  done
  stop
end
