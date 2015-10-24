#! /usr/bin/env sh
for bw in resources/*~bw.png; do
  color="${bw/~bw/~color}"
  #convert $bw -transparent white -fuzz 15% $color
  #convert $color -negate $color
  convert $bw -negate $color
  convert                        \
    $color              \
    \(                         \
       -clone 0                \
       -fill black         \
       -colorize 100           \
    \)                         \
    \(                         \
       -clone 0,1              \
       -compose difference     \
       -composite              \
       -separate               \
       +channel                \
       -evaluate-sequence max  \
       -auto-level             \
    \)                         \
     -delete 1                 \
     -alpha off                \
     -compose over             \
     -compose copy_opacity     \
     -composite                \
    $color
done
