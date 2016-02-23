if [ -f ./imgdiff_main.c ]; then
	cp /scratch/share/scws16/imgdiff/ans_1920_1080 .
	cp /scratch/share/scws16/imgdiff/ans_200_200 .
	cp /scratch/share/scws16/imgdiff/imgdiff_seq.c .
	cp /scratch/share/scws16/imgdiff/compare_matrix .
	echo "Update completed!"
else
	echo "Please execute the script in the 'imgdiff' directory"
fi
