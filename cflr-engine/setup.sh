
if [ -z "$SVF_DIR" ]; then
	echo no SVF_DIR, exiting
else
	export LLVM_DIR=$SVF_DIR/llvm-14.0.0.obj
	export Z3_DIR=$SVF_DIR/z3.obj

	echo "LLVM_DIR="$LLVM_DIR
	echo "SVF_DIR="$SVF_DIR
	echo "Z3_DIR="$Z3_DIR
fi

