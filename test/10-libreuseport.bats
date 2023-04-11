#!/usr/bin/env bats

@test "libreuseport: disable SO_REUSEPORT" {
  # Some versions of netcat enable SO_REUSEPORT
  env LD_PRELOAD=libreuseport.so SO_REUSEPORT=0 sh -c "nc -l 7873 &"
  run env LD_PRELOAD=libreuseport.so SO_REUSEPORT=0 nc -l 7873

  expected="nc: Address already in use"

  nc -z 127.0.0.1 7873
  nc -z 127.0.0.1 7873 || true

  cat <<EOF
--- output
$output
--- expected
$expected
EOF

  [ "$status" -ne 0 ]
  [ "$output" = "$expected" ]
}

@test "libreuseport: enable SO_REUSEPORT" {
  env LD_PRELOAD=libreuseport.so sh -c "nc -l 7378 & nc -l 7378 &"

  nc -z 127.0.0.1 7378
  run nc -z 127.0.0.1 7378

  cat <<EOF
--- output
$output
--- expected
EOF

  [ "$status" -eq 0 ]
}
