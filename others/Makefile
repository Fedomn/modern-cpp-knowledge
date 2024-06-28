# gdb ./a.out
# disassemble main
# see: 
# callq  0x401060 <__cyg_profile_func_enter@plt>
# callq  0x401050 <__cyg_profile_func_exit@plt>

# https://zhuanlan.zhihu.com/p/706025483
# https://gist.github.com/simonhf/489b1f3353a0ac095df01a1ddb6ad0e7
build_instr:
	g++ -finstrument-functions instr.cpp
