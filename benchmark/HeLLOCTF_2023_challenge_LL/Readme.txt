If you have other questions on the LL track, please feel free to contact Zhaokun Han: hzhk0618@tamu.edu


1. "orig" is the binary executable file (oracle) of the original design.
  With the oracle, we can test with an arbitrary input pattern with the correct input length.
  E.g., the warmup design has 36 input bits. If we would like to query with the input pattern of 111111111111111111111111111111111111, then the command line (input bits are separated by space) is 
    ./<path_to_oracle>/orig 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1
  In the terminal, it prints out
    orig
    0
  It represents that when the input pattern is 111111111111111111111111111111111111, the correct output of "orig" is 0.
  For challenge designs, the usage of oracles keeps the same.

2. The locked circuit netlist ("locked.bench") is open to the attacker. It contains primary inputs, key inputs, and output(s).

3. There are two available threat models:
  a. Oracle-based:
   The attacker can access "locked.bench" and "orig" (binary executable file) only. With the help of the oracle ("orig"), the attacker can observe the correct outputs for arbitrary input patterns.
   The attacker's goal is to either retrieve the correct key or generate a recovered design (netlist) with the same functionality as the oracle.
  b. Oracle less:
   The attacker only has access to the locked design with netlist detail ("locked.bench"). 
   The attacker's goal is to either retrieve the correct key or generate a recovered design (netlist).
