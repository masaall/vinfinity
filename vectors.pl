#!/usr/bin/perl -w

for (my $i = 0; $i < 256; $i++){
	print "vector$i: \n";
	if (!($i == 8 || ($i >= 10 && $i <= 14) || $i == 17)){
		print " pushq \$0\n";
	}
	print " pushq \$$i\n";
	print " jmp isr_entry\n";
}

print ".globl vectors\n";
print "vectors: \n";
for (my $i = 0; $i < 256; $i++){
	print ".quad vector$i\n";
}
