#!/usr/bin/perl

# Operator record lines (see LAO/LIR/Operator.src file):
#
# line 1   -- operator name
# line 2   -- operator syntax
# line 3   -- ??
# line 4   -- operator assembly format
# line 5   -- operator attributes
# line 6   -- operator operand count
# line 7   -- operator predicate index (-1 means not predicated)
# line 8   -- operator target index (for branches)
# line 9   --  
# line 10  -- operator addressing mode (may be empty)
# line 11  -- load/store base operand index (-1 if not applicable)
# line 12  -- load/store offset operand index (-1 if not applicable)
# line 13 -- access width (0 for not load/store)
# line 14 -- access alignment (0 for not load/store)
# 13 --
# 14 --
# line 15  -- operator operand types
# line 16  -- operator result types
# line 17 -- relocation type
# 17 --

# ==================================================================
#    sort_by_attributes ()
#
#    It is not enough at this point to parse just the attributes
#    line. I need to parse the semantics file also, which for the
#    moment is LAO/LIR/SemTrees.src
# ==================================================================

sub sort_by_attributes {

    my $attributes = $OP_attr[$Opcode];

    printf (STDOUT "  *** sorting attributes *** \n");

    # Some special cases that are not represented in Operator.ent
    # well:
#    if ($OP_opcode[$Opcode] eq 'GP16_GOTO_G0F_S9') {
#	push(@{$AttrGroup{'gp16'}}, $OP_opcode[$Opcode]);
#	push(@{$AttrGroup{'predicated'}}, $OP_opcode[$Opcode]);
#	push(@{$AttrGroup{'augmi'}}, $OP_opcode[$Opcode]);
#	push(@{$AttrGroup{'branch'}}, $OP_opcode[$Opcode]);
#	$OP_subset[$Opcode] = "gp16";
#	push(@{$SUBSET_opcodes[&subset_id("gp16")]}, $OP_opcode[$Opcode]);
#    }
#    elsif ($OP_opcode[$Opcode] eq 'GP16_GOTO_S11') {
#	push(@{$AttrGroup{'gp16'}}, $OP_opcode[$Opcode]);
#	push(@{$AttrGroup{'augmi'}}, $OP_opcode[$Opcode]);
#	push(@{$AttrGroup{'jump'}}, $OP_opcode[$Opcode]);
#	$OP_subset[$Opcode] = "gp16";
#	push(@{$SUBSET_opcodes[&subset_id("gp16")]}, $OP_opcode[$Opcode]);
#    }
#    elsif ($OP_opcode[$Opcode] eq 'GP16_BRANCH') {
#	push(@{$AttrGroup{'gp16'}}, $OP_opcode[$Opcode]);
#	push(@{$AttrGroup{'augmi'}}, $OP_opcode[$Opcode]);
#	push(@{$AttrGroup{'ijump'}}, $OP_opcode[$Opcode]);
#	$OP_subset[$Opcode] = "gp16";
#	push(@{$SUBSET_opcodes[&subset_id("gp16")]}, $OP_opcode[$Opcode]);
#    }
#    elsif ($OP_opcode[$Opcode] eq 'GP16_BRANCH_G0F') {
#	push(@{$AttrGroup{'gp16'}}, $OP_opcode[$Opcode]);
#	push(@{$AttrGroup{'predicated'}}, $OP_opcode[$Opcode]);
#	push(@{$AttrGroup{'augmi'}}, $OP_opcode[$Opcode]);
#	push(@{$AttrGroup{'branch'}}, $OP_opcode[$Opcode]);
#	$OP_subset[$Opcode] = "gp16";
#	push(@{$SUBSET_opcodes[&subset_id("gp16")]}, $OP_opcode[$Opcode]);
#    }
#    elsif ($OP_opcode[$Opcode] eq 'GP16_JUMP') {
#	push(@{$AttrGroup{'gp16'}}, $OP_opcode[$Opcode]);
#	push(@{$AttrGroup{'augmi'}}, $OP_opcode[$Opcode]);
#	push(@{$AttrGroup{'ijump'}}, $OP_opcode[$Opcode]);
#	$OP_subset[$Opcode] = "gp16";
#	push(@{$SUBSET_opcodes[&subset_id("gp16")]}, $OP_opcode[$Opcode]);
#    }
#
    if ($OP_opcode[$Opcode] eq 'GP32_GOTO_GF_S21') {
	push(@{$AttrGroup{'gp32'}}, $OP_opcode[$Opcode]);
	push(@{$AttrGroup{'predicated'}}, $OP_opcode[$Opcode]);
	push(@{$AttrGroup{'branch'}}, $OP_opcode[$Opcode]);
	$OP_subset[$Opcode] = "gp32";
	push(@{$SUBSET_opcodes[&subset_id("gp32")]}, $OP_opcode[$Opcode]);
    }
    elsif ($OP_opcode[$Opcode] eq 'GP32_GOTO_S25') {
	push(@{$AttrGroup{'gp32'}}, $OP_opcode[$Opcode]);
	push(@{$AttrGroup{'jump'}}, $OP_opcode[$Opcode]);
	$OP_subset[$Opcode] = "gp32";
	push(@{$SUBSET_opcodes[&subset_id("gp32")]}, $OP_opcode[$Opcode]);
    }
    elsif ($OP_opcode[$Opcode] eq 'GP32_BRANCH') {
	push(@{$AttrGroup{'gp32'}}, $OP_opcode[$Opcode]);
	push(@{$AttrGroup{'ijump'}}, $OP_opcode[$Opcode]);
	$OP_subset[$Opcode] = "gp32";
	push(@{$SUBSET_opcodes[&subset_id("gp32")]}, $OP_opcode[$Opcode]);
    }
    elsif ($OP_opcode[$Opcode] eq 'GP32_BRANCH_GF') {
	push(@{$AttrGroup{'gp32'}}, $OP_opcode[$Opcode]);
	push(@{$AttrGroup{'predicated'}}, $OP_opcode[$Opcode]);
	push(@{$AttrGroup{'branch'}}, $OP_opcode[$Opcode]);
	$OP_subset[$Opcode] = "gp32";
	push(@{$SUBSET_opcodes[&subset_id("gp32")]}, $OP_opcode[$Opcode]);
    }
    elsif ($OP_opcode[$Opcode] eq 'GP32_JUMP') {
	push(@{$AttrGroup{'gp32'}}, $OP_opcode[$Opcode]);
	push(@{$AttrGroup{'ijump'}}, $OP_opcode[$Opcode]);
	$OP_subset[$Opcode] = "gp32";
	push(@{$SUBSET_opcodes[&subset_id("gp32")]}, $OP_opcode[$Opcode]);
    }
    elsif ($OP_opcode[$Opcode] eq 'GP32_JUMP_GF') {
	push(@{$AttrGroup{'gp32'}}, $OP_opcode[$Opcode]);
	push(@{$AttrGroup{'predicated'}}, $OP_opcode[$Opcode]);
	push(@{$AttrGroup{'branch'}}, $OP_opcode[$Opcode]);
	$OP_subset[$Opcode] = "gp32";
	push(@{$SUBSET_opcodes[&subset_id("gp32")]}, $OP_opcode[$Opcode]);
    }
    elsif ($OP_opcode[$Opcode] eq 'GP32_GOTOPR_GF_U16') {
	push(@{$AttrGroup{'gp32'}}, $OP_opcode[$Opcode]);
	push(@{$AttrGroup{'predicated'}}, $OP_opcode[$Opcode]);
	push(@{$AttrGroup{'branch'}}, $OP_opcode[$Opcode]);
	$OP_subset[$Opcode] = "gp32";
	push(@{$SUBSET_opcodes[&subset_id("gp32")]}, $OP_opcode[$Opcode]);
    }
    elsif ($OP_opcode[$Opcode] eq 'GP32_GOTOPR_U16') {
	push(@{$AttrGroup{'gp32'}}, $OP_opcode[$Opcode]);
	push(@{$AttrGroup{'ijump'}}, $OP_opcode[$Opcode]);
	$OP_subset[$Opcode] = "gp32";
	push(@{$SUBSET_opcodes[&subset_id("gp32")]}, $OP_opcode[$Opcode]);
    }

    # Now just parse the Operator.ent file:
    else {

    foreach my $attribute (split(/\|/,$attributes)) {
	if ($attribute =~ /OpAtt_(\w*)/) {
        }
	else {
	    print "ERROR: corrupted attribute list !\n";
	    exit(1);
	}

	if ($1 eq "GP32") { 
	    push(@{$AttrGroup{'gp32'}}, $OP_opcode[$Opcode]);
	    $OP_subset[$Opcode] = "gp32";
	    push(@{$SUBSET_opcodes[&subset_id("gp32")]}, $OP_opcode[$Opcode]);
	}
	elsif ($1 eq "GP16") { 
	    printf (STDOUT "### Error: GP16 attribute for opcode %s\n",
		    $OP_opcode[$Opcode]);
	    exit(1);
#	    push(@{$AttrGroup{'gp16'}}, $OP_opcode[$Opcode]);
#	    $OP_subset[$Opcode] = "gp16";
#	    push (@{$SUBSET_opcodes[&subset_id("gp16")]}, $OP_opcode[$Opcode]);
	}
	elsif ($1 eq "GUARDED") {
	    push(@{$AttrGroup{'predicated'}}, $OP_opcode[$Opcode]);
	}
	elsif ($1 eq "SYNTHETIC") {
	    push(@{$AttrGroup{'synthetic'}}, $OP_opcode[$Opcode]);
	}
	elsif ($1 eq "LOAD") {
	    push(@{$AttrGroup{'load'}}, $OP_opcode[$Opcode]);
	    push(@{$MemBytes{$OP_bytes[$Opcode]}}, $OP_opcode[$Opcode]);
	    push(@{$MemAlign{$OP_align[$Opcode]}}, $OP_opcode[$Opcode]);
	    $OP_properties[$Opcode] ^= $OP_LOAD;
	}
	elsif ($1 eq "STORE") {
	    push(@{$AttrGroup{'store'}}, $OP_opcode[$Opcode]);
	    push(@{$MemBytes{$OP_bytes[$Opcode]}}, $OP_opcode[$Opcode]);
	    push(@{$MemAlign{$OP_align[$Opcode]}}, $OP_opcode[$Opcode]);
	    $OP_properties[$Opcode] ^= $OP_STORE;
	}
	elsif ($1 eq "DU_GMI") {
	    push(@{$AttrGroup{'dugmi'}}, $OP_opcode[$Opcode]);
	}
	elsif ($1 eq "AU_GMI") {
	    push(@{$AttrGroup{'augmi'}}, $OP_opcode[$Opcode]);
	}
	elsif ($1 eq "GCI") {
	    push(@{$AttrGroup{'gci'}}, $OP_opcode[$Opcode]);
	}
	elsif ($1 eq "MAC") {
	    push(@{$AttrGroup{'madd'}}, $OP_opcode[$Opcode]);
	}
	elsif ($1 eq "GOTO") {
	    push(@{$AttrGroup{'jump'}}, $OP_opcode[$Opcode]);
	}
	elsif ($1 eq "JUMP") {
	    push(@{$AttrGroup{'ijump'}}, $OP_opcode[$Opcode]);
	}
	elsif ($1 eq "BRANCH") {
	    push(@{$AttrGroup{'branch'}}, $OP_opcode[$Opcode]);
	}
	elsif ($1 eq "CALL") {
	    push(@{$AttrGroup{'call'}}, $OP_opcode[$Opcode]);
	}
	elsif ($1 eq "RETURN") {
	    push(@{$AttrGroup{'ret'}}, $OP_opcode[$Opcode]);
	}
	elsif ($1 eq "LINK") {
	    push(@{$AttrGroup{'link'}}, $OP_opcode[$Opcode]);
	}
	elsif ($1 eq "TRAP") {
	    push(@{$AttrGroup{'trap'}}, $OP_opcode[$Opcode]);
	}
	elsif ($1 eq "PACKED") {
	    push(@{$AttrGroup{'packed'}}, $OP_opcode[$Opcode]);
	}
	elsif ($1 eq "POP") {
	    push(@{$AttrGroup{'pop'}}, $OP_opcode[$Opcode]);
	}
	elsif ($1 eq "PUSH") {
	    push(@{$AttrGroup{'push'}}, $OP_opcode[$Opcode]);
	}
	elsif ($1 eq "SENSITIVE") {
	    push(@{$AttrGroup{'unsafe'}}, $OP_opcode[$Opcode]);
	}
	elsif ($1 eq "AUTOMOD") {
	    # this will be taken care of by AddrMode processing
	    ;
	}
	elsif ($1 eq "LAO") {
	    push(@{$AttrGroup{'lao'}}, $OP_opcode[$Opcode]);
	}
	elsif ($1 eq "HLIB") {
	    push(@{$AttrGroup{'hlib'}}, $OP_opcode[$Opcode]);
	}
	elsif ($1 eq "LOOPSTART") {
	    push(@{$AttrGroup{'loopstart'}}, $OP_opcode[$Opcode]);
	}
	elsif ($1 eq "LOOPEND") {
	    push(@{$AttrGroup{'loopend'}}, $OP_opcode[$Opcode]);
	}
	elsif ($1 eq "PARTIAL") {
	    push(@{$AttrGroup{'partial'}}, $OP_opcode[$Opcode]);
	}
	else {
	    print "ERROR: unknown attribute $1\n";
	    exit(1);
	}
    } # foreach attribute

    } # finish parsing the Operator.ent file

    # sanity check
    if (!defined($OP_subset[$Opcode])) {
	printf (STDOUT "ERROR: unknown subset for %s\n", $OP_opcode[$Opcode]);
	exit(1);
    }

#    print "AttrGroups : \n";
#    foreach $group (keys(%AttrGroup)) {
#	print "$group: ";
#	foreach $attr (@{$AttrGroup{$group}}) {
#	    print "$attr, ";
#	}
#	print "\n";
#    }

}

# ==================================================================
#    get_type_with_index (index)
#
#    It is not enough at this point to parse just the attributes
#    line. I need to parse the semantics file also, which for the
#    moment is LAO/LIR/SemTrees.src
# ==================================================================

sub get_type_with_index {

    my $operand;
    my $count = 1;
    my $type;
    my $operand_index = $_[0];
    my $kind = $_[1];

    $OP_opcode[$Opcode] =~ /([a-zA-Z0-9]*)_([a-zA-Z0-9]*)_(\w*)/;
    my @signature = split(/_/, $3);

CONTINUE:
    foreach my $opnd (@signature) {
	$operand = $opnd;

	if (($operand eq "M") ||
	    ($operand eq "P") ||
	    ($operand eq "BM") ||
	    ($operand eq "BP") ||
	    ($operand eq "QM") ||
	    ($operand eq "QP") ||
	    ($operand eq "MQ") ) {

	    # addressing mode -- ignore:
	    next CONTINUE;
	}

	if ($count == $operand_index) {
	    goto BREAK;
	}
	$count++;
    }

  BREAK:

    if ($operand eq "GT") {
	$type = "pr_predicate";
    }
    elsif ($operand eq "GF") {
	$type = "pr_predicate_reversed";
    }
    elsif ($operand eq "G0T") {
	$type = "g0_predicate";
    }
    elsif ($operand eq "G0F") {
	$type = "g0_predicate_reversed";
    }
    elsif ($operand eq "G0") {
	$type = "g0";
    }
    elsif ($operand eq "MD") {
	$type = "md";
    }
    elsif ($operand eq "U4") {
	$type = "u4";
    }
    elsif ($operand eq "U5") {
      # Some ST100 instructions, eg. ADDWA, ADDHA, LDW, LDH, etc.
      # scale their second operand. If the operand is an immediate
      # compiler can use a larger immediate that fits into U9 for
      # the ADDWA_GT_AR_AR_U9, for example, because the immediate
      # is scaled: 0x204 fits into U9 because scaled it corresponds
      # to 0x041. It corresponds to an illusion that ADDWA is able
      # to take a U11. I will create such illusion here. Ugly hack !
      # but the Operator.ent is not too well designed.
	if ($OP_opcode[$Opcode] eq 'GP32_LDH_GT_DR_AR_BM_U5' ||
	    $OP_opcode[$Opcode] eq 'GP32_LDH_GT_DR_AR_BP_U5' ||
	    $OP_opcode[$Opcode] eq 'GP32_LDH_GT_DR_AR_MQ_U5' ||
	    $OP_opcode[$Opcode] eq 'GP32_LDH_GT_DR_AR_QP_U5') {
	    $type = "u6";
	}
	elsif ($OP_opcode[$Opcode] eq 'GP32_LDW_GT_DR_AR_BM_U5' ||
	       $OP_opcode[$Opcode] eq 'GP32_LDW_GT_DR_AR_BP_U5' ||
	       $OP_opcode[$Opcode] eq 'GP32_LDW_GT_DR_AR_MQ_U5' ||
	       $OP_opcode[$Opcode] eq 'GP32_LDW_GT_DR_AR_QM_U5' ||
	       $OP_opcode[$Opcode] eq 'GP32_LDW_GT_DR_AR_QP_U5' ||
	       $OP_opcode[$Opcode] eq 'GP32_LDW_GT_MD_DR_AR_M_U5' ||
	       $OP_opcode[$Opcode] eq 'GP32_LDW_GT_MD_DR_AR_P_U5' ||
	       $OP_opcode[$Opcode] eq 'GP32_LDW_GT_MD_DR_AR_QM_U5' ||
	       $OP_opcode[$Opcode] eq 'GP32_LDW_GT_MD_DR_AR_QP_U5') {
	    $type = "u7";
	}
	else {
	  $type = "u5";
        }
    }
    elsif ($operand eq "U6") {
	$type = "u6";
    }
    elsif ($operand eq "U7") {
	$type = "u7";
    }
    elsif ($operand eq "U8") {
	$type = "u8";
    }
    elsif ($operand eq "U9") {
      # Some ST100 instructions, eg. ADDWA, ADDHA, LDW, LDH, etc.
      # scale their second operand. If the operand is an immediate
      # compiler can use a larger immediate that fits into U9 for
      # the ADDWA_GT_AR_AR_U9, for example, because the immediate
      # is scaled: 0x204 fits into U9 because scaled it corresponds
      # to 0x041. It corresponds to an illusion that ADDWA is able
      # to take a U11. I will create such illusion here. Ugly hack !
      # but the Operator.ent is not too well designed.
	if ($OP_opcode[$Opcode] eq 'GP32_LDH_GT_DR_AR_M_U9' ||
	    $OP_opcode[$Opcode] eq 'GP32_LDH_GT_DR_AR_P_U9' ||
	    $OP_opcode[$Opcode] eq 'GP32_ADDHA_GT_AR_AR_U9') {
	    $type = "u10";
	}
	elsif ($OP_opcode[$Opcode] eq 'GP32_LDW_GT_DR_AR_M_U9' ||
	       $OP_opcode[$Opcode] eq 'GP32_LDW_GT_DR_AR_P_U9' ||
	       $OP_opcode[$Opcode] eq 'GP32_ADDWA_GT_AR_AR_U9') {
	    $type = "u11";
	}
	else {
	  $type = "u9";
        }
    }
    elsif ($operand eq "U10") {
	$type = "u10";
    }
    elsif ($operand eq "U12") {
	$type = "u12";
    }
    elsif ($operand eq "U15") {
	$type = "u15";
    }
    elsif ($operand eq "U16") {
	$type = "u16";
    }
    elsif (($operand eq "U20") ||
	   ($operand eq "RSET")) {
	$type = "u20";
    }
    elsif ($operand eq "U32") {
	$type = "u32";
    }
    elsif ($operand eq "S7") {
	$type = "s7";
    }
    elsif ($operand eq "S9") {
	$type = "s9";
    }
    elsif ($operand eq "S11") {
	$type = "s11";
    }
    elsif ($operand eq "S16") {
	$type = "s16";
    }
    elsif ($operand eq "S21") {
	$type = "s21";
    }
    elsif ($operand eq "S25") {
	$type = "s25";
    }
    elsif ($operand eq "S32") {
	$type = "s32";
    }
    elsif ($operand eq "S40") {
	$type = "s40";
    }
    elsif (($operand eq "P3") ||
	   ($operand eq "PR")) {
	$type = "p3_implicit";
    }
    elsif ($operand eq "P13") {
	$type = "p13";
    }
    elsif ($operand eq "P15") {
	$type = "p15";
    }
    elsif ($operand eq "DR") {
	$type = "int40";
    }
    elsif ($operand eq "DRL") {
	$type = "int40l";
    }
    elsif ($operand eq "DRH") {
	$type = "int40h";
    }
    elsif ($operand eq "AR") {
	$type = "ptr32";
    }
    elsif ($operand eq "ARL") {
	$type = "ptr32l";
    }
    elsif ($operand eq "ARH") {
	$type = "ptr32h";
    }
    elsif ($operand eq "CR") {
	$type = "ctrl";
    }
    elsif ($operand eq "CRL") {
	$type = "ctrll";
    }
    elsif ($operand eq "CRH") {
	$type = "ctrlh";
    }
    elsif ($operand eq "BR") {
	$type = "pr";
    }
    elsif ($operand eq "LR") {
	$type = "lr";
    }
    else {
	print "ERROR: unknown operand type of $operand!\n";
	exit(1);
    }

    # add attributes if it's a use:
    if ($kind eq "use") {
	# predicate ? 
        # should be able to read from GT/GF/etc. earlier
#	if ($count-1 == $OP_pred[$Opcode]) {
#	    $type = $type."_predicate";
#	}
	# target ?
	if ($count-1 == $OP_target[$Opcode]) {
	    $type = $type."_target";
	}
	# base ?
	if ($count-1 == $OP_base[$Opcode]) {
	    $type = $type."_base";
	}
	# offset ?
	if ($count-1 == $OP_offset[$Opcode]) {
	    $type = $type."_offset";
	}
	# stored value ???
	# seems to always be the last operand.
	if ($OP_properties[$Opcode] & $OP_STORE) {
	    if ($count == $OP_opcnt[$Opcode]) {
		$type = $type."_storeval";
	    }
	}
    }

    # add def attributes:
    if ($kind eq "def") {
	# check whether it should be same_res for some operand
	my @same_res = split(/_/, $OP_operands[$Opcode]);
	foreach my $sr (@same_res) {
	    if ($sr =~ /OP(\d*)/) {
		if ($count == $1) {
		    $val = $count-1;
		    $type = $type."_same$val";
		}
	    }
	}
    }

    $type;
}

# ==================================================================
#    add_operand_type
# ==================================================================

sub add_operand_type {

    my $type;
    my $operand = $_[0];
    my $kind = $_[1];

    if ($operand =~ /OP(\d*)/) {
	$type = &get_type_with_index ($1, $kind);
    }
    elsif ($operand eq "BR0") {
	$type = "g0";
    }
    elsif ($operand eq "DR3") {
	$type = "r3";
    }
    elsif ($operand eq "AR3") {
	$type = "p3";
    }
    elsif ($operand eq "AR11") {
	$type = "p11";
    }
    elsif ($operand eq "AR15") {
	$type = "p15";
    }
    elsif ($operand eq "CR8") {
	$type = "cr8";
    }
    elsif ($operand eq "CR9") {
	$type = "cr9";
    }
    elsif ($operand eq "CR29") {
	$type = "cr29";
    }
    elsif ($operand eq "LR0") {
	$type = "lr0";
    }
    elsif ($operand eq "LR1") {
	$type = "lr1";
    }
    elsif ($operand eq "LR2") {
	$type = "lr2";
    }
    elsif ($operand =~ /SR(\d*)/) {
	# sticky flags -- ignore for now
	$type = "";
    }
    else {
	print "ERROR: unknown operand type $operand\n";
	exit(1);
    }

    $type;
}

# ==================================================================
#    sort_by_operands
#
#    sort Opcodes by their signature.
# ==================================================================

sub sort_by_operands {

    my $type;

    my @operands;
    my @oplist;
    my @results;
    my @reslist;

    printf (STDOUT "  *** sorting operands *** \n");

    # first some special cases. Operator.ent does not satisfy:
    if ($OP_opcode[$Opcode] eq 'GP32_CALL_S25') {
	push(@{$SignatureGroup{':GP32_CALL_S25:s25'}}, $OP_opcode[$Opcode]);
	return;
    }
    elsif ($OP_opcode[$Opcode] eq 'GP32_LINK_GT') {
	push(@{$SignatureGroup{':GP32_LINK_GT:pr_predicate,p3_implicit_target'}}, 
	     $OP_opcode[$Opcode]);
	return;
    }
    elsif ($OP_opcode[$Opcode] eq 'GP32_RTS_GT') {
	push(@{$SignatureGroup{':GP32_RTS_GT:pr_predicate,p11_implicit_target'}}, 
	     $OP_opcode[$Opcode]);
	return;
    }

    # results:
    @results = split(/_/, $OP_results[$Opcode]);
    foreach my $result (@results) {
	$type = &add_operand_type ($result,"def");
	if ($type ne "") {
	    push (@reslist, $type);
	}
    }

    my $ressig = join(',', @reslist);

    # operands:
    @operands = split(/_/, $OP_operands[$Opcode]);
    foreach my $operand (@operands) {
	$type = &add_operand_type ($operand,"use");
	if ($type ne "") {
	    push (@oplist, $type);
	}
    }

    my $opsig = join(',', @oplist);
    my $signature = $ressig.":*:".$opsig;
    print "  $signature\n";

    push(@{$SignatureGroup{$signature}}, $OP_opcode[$Opcode]);

    return;
}

# ==================================================================
#    get_resource_usage
# ==================================================================

sub get_resource_usage {
    my $i;
    my $subset = $_[0];
    my $scdclass = $_[1];
    my @ires = split ('_', $_[2]);

    # only care for Reservation tables beginning
    # with I0/I1 (GP32 mode):
    $i = shift(@ires);
    if (($i ne "I0") && ($i ne "I1")) {
	return;
    }

    $SUBSET_scd{$subset}{$scdclass}{'res'}{'ISSUE'} = 0;

    # this relies that g0 will always be seen before g1:
    my $seen_g0 = 0;
CONTINUE:
    while (@ires) {
	$i = shift(@ires);
	if (($i eq "A0") || ($i eq "A1")) {
	    $SUBSET_scd{$subset}{$scdclass}{'res'}{'AU'} = 0;
	}
	elsif (($i eq "D0") || ($i eq "D1")) {
	    $SUBSET_scd{$subset}{$scdclass}{'res'}{'DU'} = 0;
	}
	elsif ($i eq "GU") {
	    $SUBSET_scd{$subset}{$scdclass}{'res'}{'GU'} = 0;
	}
	elsif ($i eq "G0") {
	    # is this a B_Unit ?
	    $seen_g0 = 1;
	}
	elsif ($i eq "G1") {
	    if ($seen_g0 == 1) {
		# it is a B_Unit
		$SUBSET_scd{$subset}{$scdclass}{'res'}{'B'} = 0;
		# reset so that I know it's been taken into account:
		$seen_g0 = 0;
	    }
	    else {
		$SUBSET_scd{$subset}{$scdclass}{'res'}{'G'} = 0;
	    }
	}
	else {
	    next CONTINUE;
	}

	# mark g0 if not marked yet:
	if ($seen_g0 == 1) {
	    $SUBSET_scd{$subset}{$scdclass}{'res'}{'G'} = 0;
	}
    }
}

# ==================================================================
#    read_scdinfo
#
#    Read file ScdClass.ent. This contains scheduling groups for
#    the ISA subset gp32. 
#
#    TODO: it should contain all subsets.
# ==================================================================

sub read_scdinfo {
    my $sset;
    my $line;

    print "  *** reading schedinfo ***\n";
    if (!open (SCHED_F, "< ../src/ScdClass.ent")) {
	print "ERROR: file \"ScdClass.ent\" does not exist\n";
	exit(1);
    }
    
CONTINUE:
    for ($sset = 0; $sset < $SUBSET_count; $sset++) {
	my $subset = $SUBSET_name[$sset];

	printf (STDOUT "read_scdinfo: inspecting subset %s\n", $subset);

	# Eventually, we should read the info for each subset in a
        # different file. For now only handle GP32.

	# GP16:
	# Do not have ScdClasses for gp16 yet, just make one big 
	# ScdClass: make it use some dummy resource.
	if ($subset eq "gp16") {
	    $SUBSET_scd{$subset}{'SCD_GP16'}{'res'}{'ISSUE'} = 0;
	    next CONTINUE;
	}

	if ($subset eq "gp32") {
	    while ($line = <SCHED_F>) {    # ScdClass Name and comment
		if ($line =~ /<tr>\s*<th>(\w*)<\/th>/) {
		    $scdclass = $1;
		    goto RECORD;
		}

		printf (STDOUT "ERROR: couldn't find next record \n");

RECORD:
		printf (STDOUT "Scd Class %s\n", $scdclass);

		# Read the ScdClass information -- 10 next lines.
		my $l1 = <SCHED_F>;   # resources that this ScdClass uses
		my $l2 = <SCHED_F>;
		my $l3 = <SCHED_F>;
		my $l4 = <SCHED_F>;
		my $l5 = <SCHED_F>;
		my $l6 = <SCHED_F>;
		my $l7 = <SCHED_F>;
		my $l8 = <SCHED_F>;
		my $l9 = <SCHED_F>;
		my $l10 = <SCHED_F>;

		if (!$l10 =~ /<\/tr>/) {
		    printf (STDOUT "ERROR: corrupted \"ScdClass.ent\" file \n");
		}

		# Used resource line.
		$l1 =~ /\s*<td>(.*)<\/td>/;
		my $resources = $1;
		my $res;
		foreach $res (split (',', $resources)) {
		    # remove leading space:
		    if ($res =~ /\s*(\w*)/) {
			$res = $1;
		    }
                    # filter out Benoit's comments:
		    if ($res =~ /(\w*)<!--MR-->/) {
			$res = $1;
		    }
		    &get_resource_usage($subset, $scdclass, $res);
		} # foreach $res
	    } # while ($line = <SCHED_F>)
	} # gp32
	else {
	    printf (STDOUT "ERROR: unknown subset %s \n", $subset);
	    exit(1);
	}
    } # foreach $subset
}

# ==================================================================
#    sort_by_scdclass
#
#    Just read the scd class info
# ==================================================================

sub sort_by_scdclass {
    my $i;
    my $subset = $OP_subset[$Opcode];

    # Do not have ScdClasses for gp16 yet, just make one big ScdClass:
    if ($subset eq "gp16") {
	$OP_scdclass[$Opcode] = "SCD_GP16";
    }

    # Intrinsics are specified as a part of the ISA description.
    # They translate into a call with no side effects.
#    if (($OP_properties[$Opcode] & $OP_INTRINSIC)) {
#	push (@{$SUBSET_scd{$subset}{'SUB'}{'ops'}}, $OP_opcode[$Opcode]);
#	return;
#    }

    my $scdclass = $OP_scdclass[$Opcode];
    printf (STDOUT "  *** sorting by scdclass ... %s \n", $scdclass);

    # sanity check
    if ( !defined($SUBSET_scd{$subset}{$scdclass}) ) {
	printf (STDOUT "ERROR: sort_by_scdclass -- unknown scd class %s for %s \n", $scdclass, $OP_opcode[$Opcode]);
	exit(1);
    }

    push (@{$SUBSET_scd{$subset}{$scdclass}{'ops'}}, $OP_opcode[$Opcode]);
}

# ==================================================================
#    sort_by_format
#
#    I replace <name> in the format line by "NAME"; I also add O/R
#    pattern to the end -- these serve as the printing group keys.
# ==================================================================

sub sort_by_format {

    my $i;
    my $count = $OP_opcnt[$Opcode];
    my @operands = split(/_/, $OP_operands[$Opcode]);

    my $group;

    printf (STDOUT "  *** sorting by format *** \n");

    # handle some special cases
    if ($OP_opcode[$Opcode] eq 'GP32_POP_U20' ||
	$OP_opcode[$Opcode] eq 'GP32_PUSH_U20') {
	$group = "NAME_0x%05x(O)";
    }
    else {
	my @string;
	my $found;

	# parse the general case
	for ($i = 0; $i < $count; $i++) {

	    # form a group identifying string
	    $found = 0;
	    foreach my $operand (@operands) {
		if ($operand =~ /OP(\d*)/) {
		    if ($1 == $i+1) {
			push (@string, "O");
			$found = 1;
			goto BREAK;
		    }
		}
	    }
BREAK:
	    if ($found == 0) {
		push (@string, "R");
	    }
	}

	my $pattern = join ('_',@string);
	my $format = $OP_format[$Opcode];
	my $name = $OP_name[$Opcode];

	my @ft = split(' ',$format);
	my @new_ft;
	foreach my $elt (@ft) {
	    if ($elt ne $name) {
		push (@new_ft, $elt);
	    }
	    else {
		push (@new_ft, "NAME");
	    }
	}

	$group = join('_',@new_ft);
	$group = $group."(".$pattern.")";
    }

    print "  $group ($OP_opcode[$Opcode])\n";

    push(@{$PrintGroup{$group}}, $OP_opcode[$Opcode]);

    return;
}

# ==================================================================
#    opcode_is_simulated
# ==================================================================

sub opcode_is_simulated {
    my $opcode;
    my $gname = $_[0];

    foreach $opcode (@SimulatedOpcodes) {
	if ($gname eq $opcode) {
	    return 1;
	}
    }

    return 0;
}

# ==================================================================
#    opcode_is_dummy
# ==================================================================

sub opcode_is_dummy {
    my $opcode;
    my $gname = $_[0];

    foreach $opcode (@DummyOpcodes) {
	if ($gname eq $opcode) {
	    return 1;
	}
    }

    return 0;
}

# ==================================================================
#    emit_opcode
#
#    Write opcodes into file isa.cxx
# ==================================================================

sub emit_opcode {
    printf(ISA_F "\t\t \"$OP_opcode[$Opcode]\",");
    if (opcode_is_simulated($OP_opcode[$Opcode]) == 1) {
	printf(ISA_F "\t // simulated");
    }
    if (opcode_is_dummy($OP_opcode[$Opcode]) == 1) {
	printf(ISA_F "\t // dummy");
    }
    printf(ISA_F "\n");
}

# ==================================================================
#    emit_scdinfo
#
#    Write scheduling info into file st100_si.cxx
# ==================================================================

sub emit_scdinfo {
    my $scdclass;
    my $opcode;

    # For each ISA subset:
    my $i;
CONTINUE:
    for ($i = 0; $i < $SUBSET_count; $i++) {
	my $subset = $SUBSET_name[$i];

	# for now only one subset can be specified in xxx_si.cxx
	if ($subset ne 'gp32') {
	    next CONTINUE;
	}

	printf (SI_F "  /* ======================================================\n");
	printf (SI_F "   * Resource description for the ISA_SUBSET_%s \n",
		$subset);
	printf (SI_F "   * ======================================================\n");
	printf (SI_F "   */ \n\n");

	printf (SI_F "  Machine(\"st120\", ISA_SUBSET_%s, argc, argv); \n",
		$subset);

	# emit groups of instructions with similar resource constraints:
        foreach $scdclass (keys%{$SUBSET_scd{$subset}}) {
            printf (SI_F "\n  ///////////////////////////////////////// \n");
            printf (SI_F "  //   Instructions for Scd Class %s \n", $scdclass);
            printf (SI_F "  ///////////////////////////////////////// \n\n");

	    printf (SI_F "  Instruction_Group(\"%s\", \n", $scdclass);
            foreach $opcode (@{$SUBSET_scd{$subset}{$scdclass}{'ops'}}) {
		printf (SI_F "\t\t TOP_%s, \n", $opcode);
	    }
	    printf (SI_F "\t\t TOP_UNDEFINED); \n\n");

            # define latencies:
	    printf (SI_F "  Any_Operand_Access_Time(0); \n");
	    printf (SI_F "  Any_Result_Available_Time(1);		// ??? not sure \n");

            # define resource requirements:
	    my $rid;
	    for ($rid = 0; $rid < $RES_count; $rid++) {
		my $rname = $RES_name[$rid];
                if (defined($SUBSET_scd{$subset}{$scdclass}{'res'}{$rname})) {
                    printf (SI_F "  Resource_Requirement(res_%s, %d); \n",
		            $rname, 
                            $SUBSET_scd{$subset}{$scdclass}{'res'}{$rname});
                }
            }
	    printf (SI_F "\n");
	}

	# THIS IS A HACK:
        #
	# emit scheduling info for simulated instructions in gp32
        # subset. Since they only exist in gp32 mode for now.
        # I am not sure where this is used ? Perhaps, I should
        # treat these the same way as dummy, i.e. no scheduling
        # info required.
#	if ($subset eq 'gp32') {
#	    printf (SI_F "  Instruction_Group(\"simulated\", \n");
#	    foreach $opcode (@SimulatedOpcodes) {
#		printf (SI_F "\t\t TOP_%s, \n", $opcode);
#	    }
#	    printf (SI_F "\t\t TOP_UNDEFINED); \n\n");

	    # define latencies:
#	    printf (SI_F "  Any_Operand_Access_Time(0); \n");
#	    printf (SI_F "  Any_Result_Available_Time(1);		// ??? not sure \n");
#	    printf (SI_F "  Resource_Requirement(res_ISSUE, 0); \n");
#	    printf (SI_F "\n");
#	}
    }

    # Finish scheduling description.
    printf (SI_F "  Machine_Done(\"st120.c\"); \n\n");
}

# ==================================================================
#    emit_properties
# ==================================================================

sub emit_properties {
    my $group;
    my $opcode;

    foreach $group (keys(%AttrGroup)) {
	printf (PROP_F "  /* ====================================== */ \n");
        printf (PROP_F "  %s = ISA_Property_Create (\"%s\"); \n",
                                                            $group, $group);
	printf (PROP_F "  Instruction_Group (%s, \n", $group);
	foreach $opcode (@{$AttrGroup{$group}}) {
	    printf (PROP_F "\t\t TOP_%s, \n", $opcode);
	}
	printf (PROP_F "\t\t TOP_UNDEFINED); \n\n");
    }

    # emit memory access properties.
#    printf (PROP_F "  /* ====================================== */ \n");
#    printf (PROP_F "  /*           Load Property                */ \n");
#    printf (PROP_F "  /* ====================================== */ \n");
#    printf (PROP_F "  ISA_Load_Group (\"load\", \n");
#    for ($opcode = 0; $opcode < $OP_count; $opcode++) {
#	if ($OP_properties[$opcode] & $OP_LOAD) {
#	    printf (PROP_F "\t\t TOP_%s, \n", $OP_opcode[$opcode]);
#	}
#    }
#    printf (PROP_F "\t\t TOP_UNDEFINED); \n");
#    printf (PROP_F "\n");
#
#    printf (PROP_F "  /* ====================================== */ \n");
#    printf (PROP_F "  /*          Store Property                */ \n");
#    printf (PROP_F "  /* ====================================== */ \n");
#    printf (PROP_F "  ISA_Store_Group (\"store\", \n");
#    for ($opcode = 0; $opcode < $OP_count; $opcode++) {
#	if ($OP_properties[$opcode] & $OP_STORE) {
#	    printf (PROP_F "\t\t TOP_%s, \n", $OP_opcode[$opcode]);
#	}
#    }
#    printf (PROP_F "\t\t TOP_UNDEFINED); \n\n");
#    printf (PROP_F "\n");

    foreach $group (keys(%MemBytes)) {
	printf (PROP_F "  /* ====================================== */ \n");
	printf (PROP_F "  /*         Memory Access Size %s          */ \n",
                                                                    $group);
	printf (PROP_F "  /* ====================================== */ \n");
	printf (PROP_F "  ISA_Memory_Access (%s, \n", $group);
	foreach $opcode (@{$MemBytes{$group}}) {
	    printf (PROP_F "\t\t TOP_%s, \n", $opcode);
	}
	printf (PROP_F "\t\t TOP_UNDEFINED); \n\n");
    }

    foreach $group (keys(%MemAlign)) {
	printf (PROP_F "  /* ====================================== */ \n");
	printf (PROP_F "  /*          Memory Alignment %s           */ \n",
                                                                    $group);
	printf (PROP_F "  /* ====================================== */ \n");
	printf (PROP_F "  ISA_Memory_Alignment (%s, \n", $group);
	foreach $opcode (@{$MemAlign{$group}}) {
	    printf (PROP_F "\t\t TOP_%s, \n", $opcode);
	}
	printf (PROP_F "\t\t TOP_UNDEFINED); \n\n");
    }

    return;
}

# ==================================================================
#    emit_subsets
# ==================================================================

sub emit_subsets {
    my $opcode;

    my $i;
    for ($i = 0; $i < $SUBSET_count; $i++) {
	my $subset = $SUBSET_name[$i];
	print SUBS_F "  ISA_SUBSET $subset; \n";
    }
    print SUBS_F "\n\n";
    print SUBS_F "  ISA_Subset_Begin(\"st100\"); \n";

    for ($i = 0; $i < $SUBSET_count; $i++) {
	my $subset = $SUBSET_name[$i];
	printf (SUBS_F "  %s = ISA_Subset_Create(NULL,\"%s\"); \n\n", $subset, $subset);
	printf (SUBS_F "  /* ==================================================================== \n");
	printf (SUBS_F "   *             ST100_%s Instructions \n", $subset);
	printf (SUBS_F "   * ==================================================================== \n");
	printf (SUBS_F "   */ \n");

        printf (SUBS_F "  Instruction_Group(%s, \n", $subset);
	foreach $opcode (@{$SUBSET_opcodes[$i]}) {
	    printf (SUBS_F "\t\t TOP_%s, \n", $opcode);
	}
	printf (SUBS_F "\t\t TOP_UNDEFINED); \n\n");
    }

    return;
}

# ==================================================================
#    emit_operands
# ==================================================================

sub emit_operands {

    
    my $signature;
    my $opcode;
    my $group = 0;

    my $rests;
    my $opnds;
    my @results;
    my @operands;

    foreach $signature (keys(%SignatureGroup)) {
	($rests, $gname, $opnds) = split(":", $signature);
	print OPND_F "  /* ====================================== */ \n";
	if ($gname eq '*') {
	    print OPND_F "  Instruction_Group(\"O_$group\", \n";
	    $group++;
	}
	else {
	    print OPND_F "  Instruction_Group(\"O_$gname\", \n";
	}
	foreach $opcode (@{$SignatureGroup{$signature}}) {
	    print OPND_F "\t\t TOP_$opcode, \n";
	}
	print OPND_F "\t\t TOP_UNDEFINED); \n\n";
	@results = split(",", $rests);
	@operands = split(",", $opnds);

	my $count;
	my $result;
	my $operand;

	$count = 0;
	foreach $result (@results) {
	    my @attributes = split("_", $result);
	    my $name = shift(@attributes);
	    my $attribute;
	    if (scalar(@attributes) == 0) {
		print OPND_F "  Result ($count, $name); \n";
	    }
	    else {
		foreach $attribute (@attributes) {
		    if ($attribute =~ /same(\d*)/) {
			printf (OPND_F "  Result (%d, %s); \n", $count, $name);
			printf (OPND_F "  Same_Res (%d); \n", $1);
		    }
		    else {
			print OPND_F "  Result ($count, $name, $attribute); \n";
		    }
		}
	    }
	    $count++;
	}

	$count = 0;
	foreach $operand (@operands) {
	    my @attributes = split("_", $operand);
	    my $name = shift(@attributes);
	    my $attribute;
	    if (scalar(@attributes) == 0) {
		print OPND_F "  Operand ($count, $name); \n";
	    }
	    else {
		foreach $attribute (@attributes) {
		    print OPND_F "  Operand ($count, $name, $attribute); \n";
		}
	    }
	    $count++;
	}

	printf(OPND_F "\n");
    }

    return;
}

# ==================================================================
#    emit_printing_formats
# ==================================================================

sub emit_printing_formats {

    # First specify bundle/grouping stuff
    printf(PRNT_F "  Define_Macro(\"END_GROUP\", \";;\");\t// end-of-group marker \n");
    printf(PRNT_F "  Define_Macro(\"BEGIN_BUNDLE\", \"// {\t      %s\");\t// bundle introducer \n", "%s");
    printf(PRNT_F "  Define_Macro(\"END_BUNDLE\", \"// };\");\t// bundle terminator \n");
    printf(PRNT_F "\n");

    my $count = 0;

    foreach my $group (keys(%PrintGroup)) {
	$group =~ /(.*)\((\w*)\)/;
	my $ft = $1;
	my $pt = $2;
	my @format = split ('_',$ft);
	my @pattern = split ('_',$pt);
	my @new_format;
	my @new_pattern;
	my @args;
	my $opcode;

	foreach my $elt (@format) {

	    if ($elt eq "%R?") {
		push (@new_format, "%s");
		push (@args, "%s");
		push (@new_pattern, shift (@pattern));
	    }
	    elsif ($elt eq "%R!") {
		push (@new_format, "%s");
		push (@args, "%s");
		push (@new_pattern, shift (@pattern));
	    }
	    elsif (($elt eq "%R") ||
		   ($elt eq "%S") ||
		   ($elt eq "%M") ) {
		push (@new_format, "%s");
		push (@args, "%s");
		push (@new_pattern, shift (@pattern));
	    }
	    elsif ($elt eq "%I") {
		push (@new_format, "%s");
		push (@args, "%d");
		push (@new_pattern, shift (@pattern));
	    }
	    elsif ($elt eq "%R,") {
		push (@new_format, "%s,");
		push (@args, "%s");
		push (@new_pattern, shift (@pattern));
	    }
	    elsif ($elt eq ",%I") {
		push (@new_format, ", %s");
		push (@args, "%d");
		push (@new_pattern, shift (@pattern));
	    }
	    elsif ($elt eq "\\\\%pr5") {
		push (@new_format, "%s");
		push (@args, "%s");
		push (@new_pattern, shift (@pattern));
	    }
	    elsif ($elt eq "0x%05x") {
		push (@new_format, "%s");
		push (@args, "0x%05x");
		push (@new_pattern, shift (@pattern));
	    }
	    elsif ($elt =~ /\%/) {
		print "ERROR: unknown print format $elt !!!\n";
		exit (1);
	    }
	    elsif ($elt eq "NAME") {
		push (@new_format, "\t%s");
		push (@args, "%s");
		push (@new_pattern, "NAME");
	    }
	    else {
		# it should be OK ??
		push (@new_format, $elt);
	    }
	}

	my $new_ft = join (' ', @new_format);

	# print print group header
	print PRNT_F "  /* ================================= */ \n";
	print PRNT_F "  ISA_PRINT_TYPE print_$count; \n";
	print PRNT_F "  print_$count = ISA_Print_Type_Create(\"print_$count\", \"$new_ft\"); \n";

	my $opnd = 0;
	my $res = 0;
	foreach my $sym (@new_pattern) {
	    if ($sym eq "O") {
		printf (PRNT_F "  Operand(%d, \"%s\"); \n", $opnd, shift (@args));
		$opnd++;
	    }
	    elsif ($sym eq "R") {
		printf (PRNT_F "  Result(%d, \"%s\"); \n", $res, shift (@args));
		$res++;
	    }
	    else {
		printf (PRNT_F "  Name(\"%s\"); \n", shift (@args));
	    }
	}

	print PRNT_F "\n";
	print PRNT_F "  Instruction_Print_Group(print_$count, \n";
	foreach $opcode (@{$PrintGroup{$group}}) {
	    print PRNT_F "\t\t TOP_$opcode, \n";
	}
	print PRNT_F "\t\t TOP_UNDEFINED); \n";
	print PRNT_F "\n";

	$count++;
    }

    return;
}

# ==================================================================
#    emit_pack_info
# ==================================================================

sub emit_pack_info {
    my $group;
    my $count;

    # for now I use the same groups as operand/result description.
    # placement of operands/results is completely fake for now.
    # TODO: should reflect things like operand scaling, etc.

    printf (PACK_F "  ISA_PACK_TYPE \n\t\t");

    # for now reuse operand/result groups:
    $count = 0;
    foreach $group (keys(%SignatureGroup)) {
	my $rests;
	my $gname;
	my $opnds;

	($rests, $gname, $opnds) = split(":", $group);

	printf (PACK_F "p%d, \t// %s\n\t\t", $count, $gname);
	$count++;
    }
    printf (PACK_F "p%d; \t// %s\n\n", $count, $gname);

    # This is just a copy of what I found in the ia64/isa_pack.cxx
    printf (PACK_F "  OPND_ADJ_TYPE	no_adj; \n\n");

    # An instruction is 32-bit wide on the ST100
    printf (PACK_F "  ISA_Pack_Begin(\"st100\", 32); \n\n");

    printf (PACK_F "  /* Create the various adjustments that need to be performed between \n");
    printf (PACK_F "   * assembly language form and packed form. Note that simple shift  \n");
    printf (PACK_F "   * adjustments have been accomplished directly in the operand packing \n");
    printf (PACK_F "   * specification. \n");
    printf (PACK_F "   */ \n\n");
    printf (PACK_F "  no_adj = Create_Operand_Adjustment(\"no adjustment\", \"O_VAL\"); \n\n");

    $count = 0;
    foreach $group (keys(%SignatureGroup)) {
	my $rests;
	my $gname;
	my $opnds;
	my $opcode;

	($rests, $gname, $opnds) = split(":", $group);

	printf (PACK_F "/* =====  p%d: ===== */ \n", $count);
	printf (PACK_F "  p%d = ISA_Pack_Type_Create(\"p%d\"); \n", 
		                                         $count, $count);

	my @results = split(",", $rests);
	my @operands = split(",", $opnds);
	my $bpos = 0;

	my $result;
	my $rcount = 0;
	foreach $result (@results) {
	    my @attributes = split("_", $result);
	    my $name = shift(@attributes);
	    my $attribute;
	    if (scalar(@attributes) == 0) {
		print PACK_F "  Result($rcount, $bpos, 5); \n";
	    }
	    else {
		foreach $attribute (@attributes) {
		    print PACK_F "  Result ($rcount, $bpos, 5); \n";
		}
	    }
	    $rcount++;
	    $bpos = $rcount * 5 + 1;
	}

	my $operand;
	my $ocount = 0;
	foreach $operand (@operands) {
	    my @attributes = split("_", $operand);
	    my $name = shift(@attributes);
	    my $attribute;
	    if (scalar(@attributes) == 0) {
		print PACK_F "  Operand ($ocount, 0, $bpos, 5); \n";
	    }
	    else {
		foreach $attribute (@attributes) {
		    if ($attribute eq "predicate") {
			print PACK_F "  Operand ($ocount, 0, $bpos, 1); \n";
			$bpos += 1;
		    }
		    else {
			print PACK_F "  Operand ($ocount, 0, $bpos, 5); \n";
			$bpos += 5;
		    }
		}
	    }
	    $ocount++;
	}

	printf (PACK_F "  Instruction_Pack_Group(p%d, \n", $count);
	my $opcode;
	foreach $opcode (@{$SignatureGroup{$group}}) {
	    # do not emit for simulated/dummy opcodes
	    if (opcode_is_simulated($opcode) == 0 &&
		opcode_is_dummy($opcode) == 0) {
		printf (PACK_F "\tTOP_%s, \t 0x10000000UL, \n",
		                                          $opcode);
	    }
	}
	print PACK_F "\tTOP_UNDEFINED); \n\n";

	$count++;
    }
}

# ==================================================================
#    emit_bundle_info
# ==================================================================

sub emit_bundle_info {
    my $unit;
    my $slot;
    my $bundle;

    # Emit the Exec_Unit types - these are EXEC_SLOTs:
    printf (BUNDLE_F "  ISA_EXEC_UNIT_TYPE \n");

    for ($unit = 0; $unit < $EXEC_SLOT_count-1; $unit++) {
	printf (BUNDLE_F "\t\t    %s_Unit, \n", $EXEC_SLOT_name[$unit]);
    }
    printf (BUNDLE_F "\t\t    %s_Unit; \n", 
	                        $EXEC_SLOT_name[$EXEC_SLOT_count-1]);
    printf (BUNDLE_F "\n");

    for ($bundle = 0; $bundle < $BUNDLE_count; $bundle++) {
	my $f_id;
	my $f_begins;
	my $f_bits;

	printf (BUNDLE_F "  ISA_Bundle_Begin(\"%s\", %d); \n",
                           $BUNDLE_name[$bundle], $BUNDLE_bits[$bundle]);
	printf (BUNDLE_F "\n");

	printf (BUNDLE_F "  /* ===== Specification for bundle packing  ===== */ \n");
	printf (BUNDLE_F "  ISA_Bundle_Pack_Create(ISA_Bundle_Pack_Little_Endian); \n");
	($f_id, $f_begins, $f_bits) = split(",", $BUNDLE_field[$bundle]);
	printf (BUNDLE_F "  Pack_Template(%d, %d, %d); \n",
                                              $f_id, $f_begins, $f_bits);
        for ($slot = 0; $slot < $BUNDLE_slots[$bundle]; $slot++) {
	    ($f_id, $f_begins, $f_bits) = 
                                split(",", $BUNDLE_slot[$bundle][$slot]);
	    printf (BUNDLE_F "  Pack_Slot(%d, 0, %d, %d); \n",
                                              $f_id, $f_begins, $f_bits);
	}
	printf (BUNDLE_F "\n");
    }

    # Emit instructions to Exec_Unit mapping information.
    for ($unit = 0; $unit < $EXEC_SLOT_count; $unit++) {
	printf (BUNDLE_F "  /* ===== Specification for %s_Unit Type ===== */ \n", $EXEC_SLOT_name[$unit]);
	printf (BUNDLE_F "  %s_Unit = ISA_Exec_Unit_Type_Create(\"%s_Unit\", NULL); \n", $EXEC_SLOT_name[$unit], $EXEC_SLOT_name[$unit]);
	printf (BUNDLE_F "  Instruction_Exec_Unit_Group(%s_Unit, \n", 
	                                        $EXEC_SLOT_name[$unit]);
	my @scdclasses = split(",",$EXEC_SLOT_scds[$unit]);
	my $scdclass;
	foreach $scdclass (@scdclasses) {
	    my $opcode;
	    foreach $opcode (@{$SUBSET_scd{'gp32'}{$scdclass}{'ops'}}) {
		printf (BUNDLE_F "\t\t TOP_%s, \n", $opcode);
	    }
	}
	printf (BUNDLE_F "\t\t TOP_UNDEFINED); \n\n");
    }

    # Emit allowed bundle combinations:
    for ($bundle = 0; $bundle < $BUNDLE_count; $bundle++) {
	my $temp;
	printf (BUNDLE_F "  /* === All legal bundle orderings (%s of them) are specified below. */ \n\n", $BUNDLE_temps[$bundle]);
	for ($temp = 0; $temp < $BUNDLE_temps[$bundle]; $temp++) {
	    my $i;
	    my $name = ${BUNDLE_temp[$bundle][$temp]}{'name'};
	    my $slots = ${BUNDLE_temp[$bundle][$temp]}{'slots'};
	    my $stops = ${BUNDLE_temp[$bundle][$temp]}{'stops'};

	    printf (BUNDLE_F "  /* ===== Template 0x00 (%d) ===== */ \n",
		                                                     $temp);
	    printf (BUNDLE_F "  ISA_Bundle_Type_Create(\"%s\", \".%s\", 2); \n", $name, $name);
	    for ($i = 0; $i < $slots; $i++) {
		my $unt = ${BUNDLE_temp[$bundle][$temp]}{'slot'}[$i]{'unit'};
		my $stp = ${BUNDLE_temp[$bundle][$temp]}{'slot'}[$i]{'stop'};
		printf (BUNDLE_F "  Slot(%d, %s_Unit); \n", $i, $unt);
		if ($stp == 1) {
		    printf (BUNDLE_F "  Stop(%d); \n", $i);
		}
	    }
            printf (BUNDLE_F "\n");

#	    printf (BUNDLE_F "  ISA_Bundle_Type_Create(\"%s_\", \".%s_\", 2); \n", $name, $name);
#	    for ($i = 0; $i < $slots; $i++) {
#		my $unt = ${BUNDLE_temp[$bundle][$temp]}{'slot'}[$i]{'unit'};
#		my $stp = ${BUNDLE_temp[$bundle][$temp]}{'slot'}[$i]{'stop'};
#		printf (BUNDLE_F "  Slot(%d, %s_Unit); \n", $i, $unt);
#		if ($stp == 1) {
#		    printf (BUNDLE_F "  Stop(%d); \n", $i);
#		}
#	    }
            # HACK for ST100 because I am too lazy to declare 88
            # additional templates, Arthur.
            # what was the last stop ?
#            if ($stp == 0) {
#		printf (BUNDLE_F "  Stop(%d); \n", $slots-1);
#            }
#            printf (BUNDLE_F "\n");

	}

        # Arthur: eventually the instr->exec_unit mapping is a
        #         part of a current bundle that start with
        #         Bundle_Begin() and ends with Bundle_End().
	printf (BUNDLE_F "\n");
	printf (BUNDLE_F "  ISA_Bundle_End(); \n");
    }

}

# ==================================================================
#    emit_decode_info
# ==================================================================

sub emit_decode_info {
    my $i;
    printf (DECODE_F "  ISA_Decode_Begin(\"st100\"); \n\n");

    # a completely dummy decoding description: all units go to alu
    printf (DECODE_F "  STATE ex_unit = Create_Unit_State(\"ex_unit\", 0, 4); \n\n");
    printf (DECODE_F "  STATE alu = Create_Inst_State(\"alu\", 0, 0, 15); \n\n");

    printf (DECODE_F "  Transitions(ex_unit, \n");
    for ($i = 0; $i < $EXEC_SLOT_count; $i++) {
	printf (DECODE_F "        ISA_EXEC_%s_Unit, alu, \n", 
                                              $EXEC_SLOT_name[$i]);
    }
    printf (DECODE_F "	      END_TRANSITIONS); \n\n");

    printf (DECODE_F "  Transitions(alu, \n");
    # all instructions:
CONTINUE:
    for ($i = 0; $i < $OP_count; $i++) {
	my $opcode = $OP_opcode[$i];
        # skip if it's an simulated/dummy:
	if (($OP_properties[$i] & $OP_DUMMY) ||
	    ($OP_properties[$i] & $OP_SIMULATED)) {
#	    ($OP_properties[$i] & $OP_INTRINSIC)) {
	    next CONTINUE;
	}
	printf (DECODE_F "	   %d,\t Final(TOP_%s),\n", $i, $opcode);
    }
    printf (DECODE_F "	      END_TRANSITIONS); \n\n");

    printf (DECODE_F "  Initial_State(ex_unit); \n\n");

    printf (DECODE_F "  ISA_Decode_End(); \n");
}

# ==================================================================
#    init_required_opcodes
#
#    The Pro64 back end requires a number of Opcodes to be defined:
#
#    // Simulated instructions: these may be translated into
#    // a number of target instructions, and also may depend
#    // on an execution unit, eg. noop on IA64.
#      "asm",                    /* a ASM statement */
#      "intrncall",
#      "spadjust",               /* SP adjustment OP */
#      "copy.br",
#      "noop"
#
#    // Dummy instructions: these do not get emitted.
#      "begin.pregtn",
#      "end.pregtn",
#      "bwd.bar",
#      "fwd.bar",
#      "dfixup",
#      "ffixup",
#      "ifixup",
#      "label",
#
# ==================================================================

sub init_required_opcodes {

    @SimulatedOpcodes;
    @DummyOpcodes;

    my $opcode;

    push (@SimulatedOpcodes, "asm");
    push (@SimulatedOpcodes, "intrncall");
    push (@SimulatedOpcodes, "spadjust");
    push (@SimulatedOpcodes, "copy_br");
    push (@SimulatedOpcodes, "noop");
    push (@SimulatedOpcodes, "phi");
    push (@SimulatedOpcodes, "psi");

    push (@DummyOpcodes, "begin_pregtn");
    push (@DummyOpcodes, "end_pregtn");
    push (@DummyOpcodes, "bwd_bar");
    push (@DummyOpcodes, "fwd_bar");
    push (@DummyOpcodes, "dfixup");
    push (@DummyOpcodes, "ffixup");
    push (@DummyOpcodes, "ifixup");
    push (@DummyOpcodes, "label");

    # Add to the OP_xxx[] tables:
    foreach $opcode (@SimulatedOpcodes) {
	$OP_opcode[$OP_count] = $opcode;
	$OP_properties[$OP_count] = $OP_SIMULATED;
	$OP_count++;
    }

    foreach $opcode (@DummyOpcodes) {
	$OP_opcode[$OP_count] = $opcode;
	$OP_properties[$OP_count] = $OP_DUMMY;
	$OP_count++;
    }

    #    add required opcodes to isa subsets, so they are 
    #    also emitted by the emit_subsets.
    #
    #    TODO: now subset generating skripts do not allow one instruction
    #    to be part of more than 1 ISA subset. I need to allow this for 
    #    the simulated and dummy instructions.

    # GP32:
    push (@{$SUBSET_opcodes[&subset_id("gp32")]}, 'asm');
    push (@{$SUBSET_opcodes[&subset_id("gp32")]}, 'intrncall');
    push (@{$SUBSET_opcodes[&subset_id("gp32")]}, 'spadjust');
    push (@{$SUBSET_opcodes[&subset_id("gp32")]}, 'copy_br');
    push (@{$SUBSET_opcodes[&subset_id("gp32")]}, 'noop');
    push (@{$SUBSET_opcodes[&subset_id("gp32")]}, 'phi');
    push (@{$SUBSET_opcodes[&subset_id("gp32")]}, 'psi');

    push (@{$SUBSET_opcodes[&subset_id("gp32")]}, 'begin_pregtn');
    push (@{$SUBSET_opcodes[&subset_id("gp32")]}, 'end_pregtn');
    push (@{$SUBSET_opcodes[&subset_id("gp32")]}, 'bwd_bar');
    push (@{$SUBSET_opcodes[&subset_id("gp32")]}, 'fwd_bar');
    push (@{$SUBSET_opcodes[&subset_id("gp32")]}, 'dfixup');
    push (@{$SUBSET_opcodes[&subset_id("gp32")]}, 'ffixup');
    push (@{$SUBSET_opcodes[&subset_id("gp32")]}, 'ifixup');
    push (@{$SUBSET_opcodes[&subset_id("gp32")]}, 'label');

    # isa_properties: add required opcodes to AttrGroup, so they are 
    # also emitted by the emit_properties.

    # simulated:
    push(@{$AttrGroup{'simulated'}}, 'asm');
    push(@{$AttrGroup{'simulated'}}, 'intrncall');
    push(@{$AttrGroup{'simulated'}}, 'spadjust');
    push(@{$AttrGroup{'simulated'}}, 'copy_br');
    push(@{$AttrGroup{'simulated'}}, 'noop');
    push(@{$AttrGroup{'simulated'}}, 'phi');
    push(@{$AttrGroup{'simulated'}}, 'psi');

    # dummy:
    push(@{$AttrGroup{'dummy'}}, 'begin_pregtn');
    push(@{$AttrGroup{'dummy'}}, 'end_pregtn');
    push(@{$AttrGroup{'dummy'}}, 'bwd_bar');
    push(@{$AttrGroup{'dummy'}}, 'fwd_bar');
    push(@{$AttrGroup{'dummy'}}, 'dfixup');
    push(@{$AttrGroup{'dummy'}}, 'ffixup');
    push(@{$AttrGroup{'dummy'}}, 'ifixup');
    push(@{$AttrGroup{'dummy'}}, 'label');

    # var_operands:
    push(@{$AttrGroup{'var_opnds'}}, 'asm');
    push(@{$AttrGroup{'var_opnds'}}, 'intrncall');
    push(@{$AttrGroup{'var_opnds'}}, 'phi');
    push(@{$AttrGroup{'var_opnds'}}, 'psi');

    # isa_operands: Create SignatureGroup entry for required opcodes.

    push(@{$SignatureGroup{':asm:'}}, 'asm');
    push(@{$SignatureGroup{':intrncall:'}}, 'intrncall');
    push(@{$SignatureGroup{':asm:'}}, 'phi');
    push(@{$SignatureGroup{':asm:'}}, 'psi');

    push(@{$SignatureGroup{'ptr32:spadjust:pr_predicate,ptr32,s32'}}, 'spadjust');
    push(@{$SignatureGroup{'ptr32:copy_br:pr_predicate,ptr32'}}, 'copy_br');
    push(@{$SignatureGroup{':noop:pr_predicate'}}, 'noop');

    push(@{$SignatureGroup{':pregtn:int40,s16'}}, 'begin_pregtn');
    push(@{$SignatureGroup{':pregtn:int40,s16'}}, 'end_pregtn');
    push(@{$SignatureGroup{':barrier:'}}, 'bwd_bar');
    push(@{$SignatureGroup{':barrier:'}}, 'fwd_bar');
    push(@{$SignatureGroup{'int40:fixup:'}}, 'dfixup');
    push(@{$SignatureGroup{'ptr32:fixup:'}}, 'ffixup');
    push(@{$SignatureGroup{'int40:fixup:'}}, 'ifixup');
    push(@{$SignatureGroup{':label:pcrel'}}, 'label');

    # ??? add required OPeration scdinfo to the GP32 subset (GP16 
    # later -- in any case for now ISA subsets can not share 
    # OPerations). 
    #
}

# ==================================================================
#    initialize_isa_file
# ==================================================================

sub initialize_isa_file {

    open (ISA_F, "> isa.cxx");

    print ISA_F "// AUTOMATICALLY GENERATED FROM ARC DATA BASE !!! \n\n";
    print ISA_F "// Generate an ISA containing the instructions specified. \n";
    print ISA_F "///////////////////////////////////////////////////////// \n";
    print ISA_F "// The instructions are listed by category. The different \n";
    print ISA_F "// categories of instructions are:\n";
    print ISA_F "// \n";
    print ISA_F "//   1. DU instructions \n";
    print ISA_F "//   2. AU instructions \n";
#    print ISA_F "//   3. Intrinsic instructions \n";
    print ISA_F "// Following must be declared, back end needs them: \n";
    print ISA_F "//   4. Simulated instructions \n";
    print ISA_F "//   5. Dummy instructions \n";
    print ISA_F "// \n";
    print ISA_F "// Within each category, the instructions are in alphabetical order.\n";
    print ISA_F "// This arrangement of instructions matches the order in the ISA manual.\n";
    print ISA_F "/////////////////////////////////////\n\n\n";
    print ISA_F "#include <stddef.h>\n";
    print ISA_F "#include \"isa_gen.h\"\n\n\n";

    print ISA_F "main ()\n";
    print ISA_F "{\n";
    print ISA_F "  ISA_Create (\"st100\", \n";

    return;
}

# ==================================================================
#    finalize_isa_file
# ==================================================================

sub finalize_isa_file {

    print ISA_F "      NULL);\n";
    print ISA_F "}\n";

    close (ISA_F);
    return;
}

# ==================================================================
#    initialize_si_file
# ==================================================================

sub initialize_si_file {

    open (SI_F, "> st120_si.cxx");

    &copyright_notice (SI_F);

    printf (SI_F "//  ST100 processor scheduling information \n");
    printf (SI_F "///////////////////////////////////// \n");
    printf (SI_F "//   \n");
    printf (SI_F "//  Description:  \n");
    printf (SI_F "//  \n");
    printf (SI_F "//  Generate a scheduling description of a ST100 processor  \n");
    printf (SI_F "//  via the si_gen interface.  \n");
    printf (SI_F "//  \n");
    printf (SI_F "/////////////////////////////////////  \n\n");

    printf (SI_F "#include \"si_gen.h\" \n");
    printf (SI_F "#include \"targ_isa_subset.h\" \n");
    printf (SI_F "#include \"topcode.h\" \n\n");

    my $format = "";
    printf (SI_F "static RESOURCE ");
    my $rid;
    for ($rid = 0; $rid < $RES_count; $rid++) {
	printf (SI_F "%s res_%s", $format, $RES_name[$rid]);
	$format = ",\n	       ";
    }
    printf (SI_F "; \n\n");
    printf (SI_F "int \n");
    printf (SI_F "main (int argc, char *argv[]) \n");
    printf (SI_F "{ \n");

    for ($rid = 0; $rid < $RES_count; $rid++) {
	printf (SI_F "  res_%s = RESOURCE_Create(\"%s\", %d); \n",
		$RES_name[$rid], $RES_name[$rid], $RES_avail[$rid]);
    }

    printf (SI_F "\n");
}

# ==================================================================
#    finalize_isa_file
# ==================================================================

sub finalize_si_file {

    printf (SI_F "}\n");

    close (SI_F);
    return;
}

# ==================================================================
#    initialize_properties_file
# ==================================================================

sub initialize_properties_file {

    open (PROP_F, "> isa_properties.cxx");

    print PROP_F "//  AUTOMATICALLY GENERATED FROM ARC DATABASE !!!\n\n";
    print PROP_F "//  Generate ISA properties information \n";
    print PROP_F "/////////////////////////////////////// \n";
    print PROP_F "// The different operators are listed as having the\n";
    print PROP_F "// following ISA_PROPERTIES: \n";
    print PROP_F "// \n";
    print PROP_F "///////////////////////////////////// \n";

    print PROP_F "#include <stddef.h> \n";
    print PROP_F "#include \"topcode.h\" \n";
    print PROP_F "#include \"isa_properties_gen.h\" \n\n";

    print PROP_F "main() \n";
    print PROP_F "{ \n";
    print PROP_F "  ISA_PROPERTY \n";
    print PROP_F "    gp32,            /* a GP32 operator */ \n";
    print PROP_F "    gp16,            /* a GP16 operator */ \n";
    print PROP_F "    duadd,            /* DU add */ \n";
    print PROP_F "    dusub,            /* DU sub */ \n";
    print PROP_F "    dumul,            /* DU mul */ \n";
    print PROP_F "    dugmi,           /* DU GMI operator */ \n";
    print PROP_F "    dumisc,           /* other DU operator */ \n";
    print PROP_F "    auadd,            /* AU add */ \n";
    print PROP_F "    ausub,            /* AU sub */ \n";
    print PROP_F "    aumul,            /* AU mul */ \n";
    print PROP_F "    augmi,           /* AU GMI operator */ \n";
    print PROP_F "    aumisc,           /* other AU operator */ \n";
    print PROP_F "    load,             /* Memory load operator */ \n";
    print PROP_F "    store,   		/* Memory store operator */ \n";
    print PROP_F "    jump,		/* Goto operator */ \n";
    print PROP_F "    ijump,		/* Indirect jump operator */ \n";
    print PROP_F "    branch,           /* Conditional branch */ \n";
    print PROP_F "    call, 		/* Subprogram call operator */ \n";
    print PROP_F "    link,             /* Link ??? */ \n";
    print PROP_F "    ret,           /* return from subroutine operator */ \n";
    print PROP_F "    trap,             /* trap operator */ \n";
    print PROP_F "    madd,             /* Operator is a MAC */ \n";
    print PROP_F "    gci,              /* ?? */ \n";
    print PROP_F "    packed,           /* Packed operator */ \n";
    print PROP_F "    pop,              /* ?? operator */ \n";
    print PROP_F "    push,             /* ?? operator */ \n";
    print PROP_F "    predicated,       /* Operator is predicated */ \n";
    print PROP_F "    unalign_ld, 	/* Unaligned load operator */ \n";
    print PROP_F "    unalign_store,	/* Unaligned store operator */ \n";
    print PROP_F "    same_res,		/* Result must be same as opnd */ \n";
    print PROP_F "    noop, 		/* No-op operator */ \n";
    print PROP_F "    info,             /* ?? */ \n";
    print PROP_F "    safe,		/* Never traps -- always safe */ \n";
    print PROP_F "    unsafe,		/* Unsafe always */ \n";
    print PROP_F "    memtrap,		/* Memory trap potential */ \n";
    print PROP_F "    side_effects,	/* Instruction has side effects */ \n";
    print PROP_F "    synthetic,        /* Synthetic ?? */ \n";

    print PROP_F "    lao,              /* ?? */ \n";
    print PROP_F "    data,             /* ?? */ \n";
    print PROP_F "    hlib,             /* ?? */ \n";
    print PROP_F "    loopstart,        /* ?? */ \n";
    print PROP_F "    loopend,          /* ?? */ \n";
    print PROP_F "    partial,          /* ?? */ \n\n";

    print PROP_F "    /* Following properties must be declared, back end \n";
    print PROP_F "     * requires some opcodes of these types: \n";
    print PROP_F "     */ \n";
#    print PROP_F "    intrinsic,        /* an intrinsic */ \n";
    print PROP_F "    simulated,        /* a macro */ \n";
    print PROP_F "    dummy,            /* Instruction is pseudo */  \n";
    print PROP_F "    var_opnds;        /* Variable number of operands AND/OR results */ \n";
    print PROP_F "\n";

    print PROP_F "  ISA_Properties_Begin (\"st100\"); \n\n\n";

    print PROP_F "/* ==================================================================== \n";
    print PROP_F " *              Operator attributes descriptors \n";
    print PROP_F " * ==================================================================== \n";
    print PROP_F " */ \n\n";

    return;
}

# ==================================================================
#    finalize_properties_file
# ==================================================================

sub finalize_properties_file {

    print PROP_F "  ISA_Properties_End();\n";
    print PROP_F "  return 0;\n";
    print PROP_F "}\n";

    close (PROP_F);
    return;
}

# ==================================================================
#    initialize_subset_file
# ==================================================================

sub initialize_subset_file {

    open (SUBS_F, "> isa_subset.cxx");

    print SUBS_F "//  AUTOMATICALLY GENERATED FROM ARC DATABASE !!! \n";
    print SUBS_F "//  Generate ISA subset descriptions \n";
    print SUBS_F "///////////////////////////////////// \n";
    print SUBS_F "// The different categories of instructions are: \n";
    print SUBS_F "// \n";
    print SUBS_F "//   1. GP32 instructions; \n";
    print SUBS_F "//   2. GP16 instructions; \n";
    print SUBS_F "// \n";
    print SUBS_F "// as shown in the ISA manual \n";
    print SUBS_F "///////////////////////////////////// \n\n";

    print SUBS_F "#include <stddef.h> \n";
    print SUBS_F "#include \"topcode.h\" \n";
    print SUBS_F "#include \"isa_subset_gen.h\" \n\n";

    print SUBS_F "main() \n";
    print SUBS_F "{ \n";

    return;
}

# ==================================================================
#    finalize_subset_file
# ==================================================================

sub finalize_subset_file {

    print SUBS_F "  ISA_Subset_End(); \n";
    print SUBS_F "  return 0; \n";
    print SUBS_F "} \n";

    close (SUBS_F);

    return;
}

# ==================================================================
#    initialize_operands_file
# ==================================================================

sub initialize_operands_file {

    open (OPND_F, "> isa_operands.cxx");

    print OPND_F "//  AUTOMATICALLY GENERATED FROM ARC DATABASE !!! \n";
    print OPND_F "// Group TOPS with similar operands/results format. \n";
    print OPND_F "///////////////////////////////////////////////////////// \n";
    print OPND_F "// Within each category, the instructions are arranged roughly in order \n";
    print OPND_F "// of increasing numbers of operands. \n";
    print OPND_F "///////////////////////////////////// \n\n\n";

    print OPND_F "#include <stddef.h> \n";
    print OPND_F "#include \"topcode.h\" \n";
    print OPND_F "#include \"isa_operands_gen.h\" \n\n";

    print OPND_F "main() \n";
    print OPND_F "{ \n\n";

    print OPND_F "  OPERAND_VALUE_TYPE int40, ptr32; \n";
    print OPND_F "  OPERAND_VALUE_TYPE lr, lr0, lr1, lr2; \n";
    print OPND_F "  OPERAND_VALUE_TYPE p3, p11, p13, p15; \n";
    print OPND_F "  OPERAND_VALUE_TYPE pr, g0; \n";
    print OPND_F "  OPERAND_VALUE_TYPE ctrl, ctrll, ctrlh, cr8, cr9, cr29; \n";
    print OPND_F "  OPERAND_VALUE_TYPE int40l, int40h, r3; \n";
    print OPND_F "  OPERAND_VALUE_TYPE ptr32l, ptr32h; \n";
    print OPND_F "  OPERAND_VALUE_TYPE md; \n";
    print OPND_F "  OPERAND_VALUE_TYPE u4, u5, u6, u7, u8, u9, u10, u11, u12, u15, u16, u20, u32; \n";
    print OPND_F "  OPERAND_VALUE_TYPE s7, s9, s11, s16, s21, s25, s32, s40; \n";
    print OPND_F "  OPERAND_VALUE_TYPE absadr, pcrel; \n";
    print OPND_F "\n";

    printf (OPND_F "  /* ------------------------------------------------------\n");
    printf (OPND_F "   *   Following built-in use types must be specified: \n");
    printf (OPND_F "   *     1. base operand use for TOP_load, TOP_store; \n");
    printf (OPND_F "   *     2. offset operand use for TOP_load, TOP_store; \n");
    printf (OPND_F "   *     3. storeval operand use for TOP_store; \n");
    printf (OPND_F "   * \n");
    printf (OPND_F "   *   Following built-in use types may be specified: \n");
    printf (OPND_F "   *     4. implicit operand use for TOPs when the operand is implicitely used; \n");
    printf (OPND_F "   * \n");
    printf (OPND_F "   *   Here you can specify any additional operand uses. \n");
    printf (OPND_F "   * ------------------------------------------------------\n");
    printf (OPND_F "   */\n");

    # These use types are required by the Pro64 cg:
    print OPND_F "  OPERAND_USE_TYPE \n";
    print OPND_F "	  predicate,	// a qualifying predicate \n";
    print OPND_F "	  reversed,	// predicate is negated\n";
#    print OPND_F "	  base,		// a base address (memory insts) \n";
#    print OPND_F "	  offset,	// an offset added to a base (imm) \n";
    print OPND_F "	  postincr,	// a post increment applied to a base address \n";
    print OPND_F "	  target,	// the target of a branch \n";
#    print OPND_F "	  storeval,	// value to be stored \n";

    # These are because the ISA can have this in any order
    print OPND_F "	  opnd1,	// first/left operand of an alu operator \n";
    print OPND_F "        opnd2;	// second/right operand of an alu operator \n";

    # These use types are ST100 specific:

#    print OPND_F "        implicit;     // implicitely used by instruction \n";
    print OPND_F "\n";

    print OPND_F "  ISA_Operands_Begin(\"st100\"); \n";

    print OPND_F "  /* Create the register operand types: */ \n";

    print OPND_F "  pr = ISA_Reg_Opnd_Type_Create(\"pr\", \n";
    print OPND_F "                ISA_REGISTER_CLASS_guard, \n";
    print OPND_F "		  ISA_REGISTER_SUBCLASS_UNDEFINED, \n";
    print OPND_F "		  1, UNSIGNED, INVALID); \n";
    print OPND_F "  lr = ISA_Reg_Opnd_Type_Create(\"lr\", \n";
    print OPND_F "                ISA_REGISTER_CLASS_loop, \n";
    print OPND_F "		  ISA_REGISTER_SUBCLASS_UNDEFINED, \n";
    print OPND_F "		  1, UNSIGNED, INVALID); \n";
    print OPND_F "  lr0 = ISA_Reg_Opnd_Type_Create(\"lr0\", \n";
    print OPND_F "                ISA_REGISTER_CLASS_loop, \n";
    print OPND_F "		  ISA_REGISTER_SUBCLASS_lr0, \n";
    print OPND_F "		  1, UNSIGNED, INVALID); \n";
    print OPND_F "  lr1 = ISA_Reg_Opnd_Type_Create(\"lr1\", \n";
    print OPND_F "                ISA_REGISTER_CLASS_loop, \n";
    print OPND_F "		  ISA_REGISTER_SUBCLASS_lr1, \n";
    print OPND_F "		  1, UNSIGNED, INVALID); \n";
    print OPND_F "  lr2 = ISA_Reg_Opnd_Type_Create(\"lr2\", \n";
    print OPND_F "                ISA_REGISTER_CLASS_loop, \n";
    print OPND_F "		  ISA_REGISTER_SUBCLASS_lr2, \n";
    print OPND_F "		  1, UNSIGNED, INVALID); \n";
    print OPND_F "  g0 = ISA_Reg_Opnd_Type_Create(\"g0\", \n";
    print OPND_F "                ISA_REGISTER_CLASS_guard, \n";
    print OPND_F "		  ISA_REGISTER_SUBCLASS_g0, \n";
    print OPND_F "		  1, UNSIGNED, INVALID); \n";
    print OPND_F "  int40 = ISA_Reg_Opnd_Type_Create(\"int40\", \n";
    print OPND_F "                ISA_REGISTER_CLASS_du, \n";
    print OPND_F "		  ISA_REGISTER_SUBCLASS_UNDEFINED, \n";
    print OPND_F "		  40, SIGNED, INVALID); \n";
    print OPND_F "  r3 = ISA_Reg_Opnd_Type_Create(\"r3\", \n";
    print OPND_F "                ISA_REGISTER_CLASS_du, \n";
    print OPND_F "		  ISA_REGISTER_SUBCLASS_r3, \n";
    print OPND_F "		  40, SIGNED, INVALID); \n";
    print OPND_F "  ptr32 = ISA_Reg_Opnd_Type_Create(\"ptr32\", \n";
    print OPND_F "                ISA_REGISTER_CLASS_au, \n";
    print OPND_F "		  ISA_REGISTER_SUBCLASS_UNDEFINED, \n";
    print OPND_F "		  32, SIGNED, INVALID); \n";
    print OPND_F "  p3 = ISA_Reg_Opnd_Type_Create(\"p3\", \n";
    print OPND_F "                ISA_REGISTER_CLASS_au, \n";
    print OPND_F "		  ISA_REGISTER_SUBCLASS_p3, \n";
    print OPND_F "		  32, UNSIGNED, INVALID); \n";
    print OPND_F "  p11 = ISA_Reg_Opnd_Type_Create(\"p11\", \n";
    print OPND_F "                ISA_REGISTER_CLASS_au, \n";
    print OPND_F "		  ISA_REGISTER_SUBCLASS_p11, \n";
    print OPND_F "		  32, UNSIGNED, INVALID); \n";
    print OPND_F "  p13 = ISA_Reg_Opnd_Type_Create(\"p13\", \n";
    print OPND_F "                ISA_REGISTER_CLASS_au, \n";
    print OPND_F "		  ISA_REGISTER_SUBCLASS_p13, \n";
    print OPND_F "		  32, UNSIGNED, INVALID); \n";
    print OPND_F "  p15 = ISA_Reg_Opnd_Type_Create(\"p15\", \n";
    print OPND_F "                ISA_REGISTER_CLASS_au, \n";
    print OPND_F "		  ISA_REGISTER_SUBCLASS_p15, \n";
    print OPND_F "		  32, UNSIGNED, INVALID); \n";
    print OPND_F "  cr8 = ISA_Reg_Opnd_Type_Create(\"cr8\", \n";
    print OPND_F "                ISA_REGISTER_CLASS_control, \n";
    print OPND_F "		  ISA_REGISTER_SUBCLASS_cr8, \n";
    print OPND_F "		  32, UNSIGNED, INVALID); \n";
    print OPND_F "  cr9 = ISA_Reg_Opnd_Type_Create(\"cr9\", \n";
    print OPND_F "                ISA_REGISTER_CLASS_control, \n";
    print OPND_F "		  ISA_REGISTER_SUBCLASS_cr9, \n";
    print OPND_F "		  32, UNSIGNED, INVALID); \n";
    print OPND_F "  cr29 = ISA_Reg_Opnd_Type_Create(\"cr29\", \n";
    print OPND_F "                ISA_REGISTER_CLASS_control, \n";
    print OPND_F "		  ISA_REGISTER_SUBCLASS_cr29, \n";
    print OPND_F "		  32, UNSIGNED, INVALID); \n";
    print OPND_F "  ctrl = ISA_Reg_Opnd_Type_Create(\"ctrl\", \n";
    print OPND_F "                ISA_REGISTER_CLASS_control, \n";
    print OPND_F "		  ISA_REGISTER_SUBCLASS_UNDEFINED, \n";
    print OPND_F "		  32, UNSIGNED, INVALID); \n";
    print OPND_F "  int40l = ISA_Reg_Opnd_Type_Create(\"int40_l\", \n";
    print OPND_F "                ISA_REGISTER_CLASS_du, \n";
    print OPND_F "		  ISA_REGISTER_SUBCLASS_drl, \n";
    print OPND_F "		  40, SIGNED, INVALID); \n";
    print OPND_F "  int40h = ISA_Reg_Opnd_Type_Create(\"int40_h\", \n";
    print OPND_F "                ISA_REGISTER_CLASS_du, \n";
    print OPND_F "		  ISA_REGISTER_SUBCLASS_drh, \n";
    print OPND_F "		  40, SIGNED, INVALID); \n";
    print OPND_F "  ptr32l = ISA_Reg_Opnd_Type_Create(\"ptr32_l\", \n";
    print OPND_F "                ISA_REGISTER_CLASS_au, \n";
    print OPND_F "		  ISA_REGISTER_SUBCLASS_arl, \n";
    print OPND_F "		  32, SIGNED, INVALID); \n";
    print OPND_F "  ptr32h = ISA_Reg_Opnd_Type_Create(\"ptr32_h\", \n";
    print OPND_F "                ISA_REGISTER_CLASS_au, \n";
    print OPND_F "		  ISA_REGISTER_SUBCLASS_arh, \n";
    print OPND_F "		  32, SIGNED, INVALID); \n";
    print OPND_F "  ctrll = ISA_Reg_Opnd_Type_Create(\"ctrl_l\", \n";
    print OPND_F "                ISA_REGISTER_CLASS_control, \n";
    print OPND_F "		  ISA_REGISTER_SUBCLASS_crl, \n";
    print OPND_F "		  32, UNSIGNED, INVALID); \n";
    print OPND_F "  ctrlh = ISA_Reg_Opnd_Type_Create(\"ctrl_h\", \n";
    print OPND_F "                ISA_REGISTER_CLASS_control, \n";
    print OPND_F "		  ISA_REGISTER_SUBCLASS_crh, \n";
    print OPND_F "		  32, UNSIGNED, INVALID); \n";
    print OPND_F "\n";

    print OPND_F "  /* Create the enum operand types: */ \n";
    print OPND_F "\n";
    print OPND_F "  md = ISA_Enum_Opnd_Type_Create(\"md\", 8, UNSIGNED, EC_amod); \n";
    print OPND_F "\n";

    print OPND_F "  /* Create the literal operand types: */ \n";
    print OPND_F "\n";
    print OPND_F "  u4   = ISA_Lit_Opnd_Type_Create(\"u4\",   4, UNSIGNED, LC_u4); \n";
    print OPND_F "  u5   = ISA_Lit_Opnd_Type_Create(\"u5\",   5, UNSIGNED, LC_u5); \n";
    print OPND_F "  u6   = ISA_Lit_Opnd_Type_Create(\"u6\",   6, UNSIGNED, LC_u6); \n";
    print OPND_F "  u7   = ISA_Lit_Opnd_Type_Create(\"u7\",   7, UNSIGNED, LC_u7); \n";
    print OPND_F "  u8   = ISA_Lit_Opnd_Type_Create(\"u8\",   8, UNSIGNED, LC_u8); \n";
    print OPND_F "  u9   = ISA_Lit_Opnd_Type_Create(\"u9\",   9, UNSIGNED, LC_u9); \n";
    print OPND_F "  u10   = ISA_Lit_Opnd_Type_Create(\"u10\",   10, UNSIGNED, LC_u10); \n";
    print OPND_F "  u11   = ISA_Lit_Opnd_Type_Create(\"u11\",   11, UNSIGNED, LC_u11); \n";
    print OPND_F "  u12   = ISA_Lit_Opnd_Type_Create(\"u12\",   12, UNSIGNED, LC_u12); \n";
    print OPND_F "  u15   = ISA_Lit_Opnd_Type_Create(\"u15\",   15, UNSIGNED, LC_u15); \n";
    print OPND_F "  u16   = ISA_Lit_Opnd_Type_Create(\"u16\",   16, UNSIGNED, LC_u16); \n";
    print OPND_F "  u20   = ISA_Lit_Opnd_Type_Create(\"u20\",   20, UNSIGNED, LC_u20); \n";
    print OPND_F "  u32   = ISA_Lit_Opnd_Type_Create(\"u32\",   32, UNSIGNED, LC_u32); \n";

    print OPND_F "  s7   = ISA_Lit_Opnd_Type_Create(\"s7\",   7, SIGNED, LC_s7); \n";
    print OPND_F "  s9   = ISA_Lit_Opnd_Type_Create(\"s9\",   9, SIGNED, LC_s9); \n";
    print OPND_F "  s11   = ISA_Lit_Opnd_Type_Create(\"s11\",   11, SIGNED, LC_s11); \n";
    print OPND_F "  s16   = ISA_Lit_Opnd_Type_Create(\"s16\",   16, SIGNED, LC_s16); \n";
    print OPND_F "  s21   = ISA_Lit_Opnd_Type_Create(\"s21\",   21, SIGNED, LC_s21); \n";
    print OPND_F "  s25   = ISA_Lit_Opnd_Type_Create(\"s25\",   25, SIGNED, LC_s25); \n";
    print OPND_F "  s32   = ISA_Lit_Opnd_Type_Create(\"s32\",   32, SIGNED, LC_s32); \n";
    print OPND_F "  s40   = ISA_Lit_Opnd_Type_Create(\"s40\",   40, SIGNED, LC_s40); \n";

    print OPND_F "  pcrel   = ISA_Lit_Opnd_Type_Create(\"pcrel\",   32, PCREL, LC_s32); \n";
    print OPND_F "  absadr   = ISA_Lit_Opnd_Type_Create(\"absadr\",   32, UNSIGNED, LC_u32); \n";

    print OPND_F "\n";

    print OPND_F "  /* Create the operand uses: */ \n";
    print OPND_F "\n";

    print OPND_F "  predicate  = Create_Operand_Use(\"predicate\"); \n";
    print OPND_F "  reversed   = Create_Operand_Use(\"reversed\"); \n";
#    print OPND_F "  base       = Create_Operand_Use(\"base\"); \n";
#    print OPND_F "  offset     = Create_Operand_Use(\"offset\"); \n";
    print OPND_F "  postincr   = Create_Operand_Use(\"postincr\"); \n";
    print OPND_F "  target     = Create_Operand_Use(\"target\"); \n";
#    print OPND_F "  storeval   = Create_Operand_Use(\"storeval\"); \n";
    print OPND_F "  opnd1      = Create_Operand_Use(\"opnd1\"); \n";
    print OPND_F "  opnd2      = Create_Operand_Use(\"opnd2\"); \n";
#    print OPND_F "  implicit   = Create_Operand_Use(\"implicit\"); \n";

    print OPND_F "\n";

    return;
}

# ==================================================================
#    finalize_operands_file
# ==================================================================

sub finalize_operands_file {

    print OPND_F "\n\n";
    print OPND_F "  ISA_Operands_End(); \n";
    print OPND_F "  return 0; \n";
    print OPND_F "} \n";

    close (OPND_F);
    return;
}

# ==================================================================
#    initialize_print_file
# ==================================================================

sub initialize_print_file {

    open (PRNT_F, "> isa_print.cxx");

    print PRNT_F "//  AUTOMATICALLY GENERATED FROM ARC DATABASE !!! \n";
    print PRNT_F "// Group TOPS with similar printing format. \n";
    print PRNT_F "//////////////////////////////////////////////////\n\n\n";

    print PRNT_F "#include <stdio.h> \n";
    print PRNT_F "#include <stddef.h> \n";
    print PRNT_F "#include <string.h> \n";
    print PRNT_F "#include <ctype.h> \n";
    print PRNT_F "#include \"topcode.h\" \n";
    print PRNT_F "#include \"isa_print_gen.h\" \n\n";

    print PRNT_F "// Multiple topcodes map to the same assembly name. To disambiguate the \n";
    print PRNT_F "// topcodes, we append a signature to the basename. To get the assembly \n";
    print PRNT_F "// name we must strip off the suffix. \n\n";

    print PRNT_F "static const char *asmname(TOP topcode) \n";
    print PRNT_F "{ \n";
    print PRNT_F "  int c; \n";
    print PRNT_F "  int i; \n";
    print PRNT_F "  int j; \n";
    print PRNT_F "  int k; \n";
    print PRNT_F "  const char *name = TOP_Name(topcode); \n";
    print PRNT_F "  char buf[100]; \n";
    print PRNT_F "  char lower_buf[100]; \n\n";

    print PRNT_F "  /* \n";
    print PRNT_F "   * First handle simulated and dummy instructions: \n";
    print PRNT_F "   */ \n";
    print PRNT_F "  switch (topcode) { \n";

    my $opcode;
    foreach $opcode (@SimulatedOpcodes) {
	printf (PRNT_F "  case TOP_%s: return \"%s\"; \n", $opcode, $opcode);
    }

    foreach $opcode (@DummyOpcodes) {
	printf (PRNT_F "  case TOP_%s: return \"%s\"; \n", $opcode, $opcode);
    }
    print PRNT_F "  default: break; \n";
    print PRNT_F "  } \n";
    print PRNT_F "\n";

    print PRNT_F "  /* \n";
    print PRNT_F "   * By convention we have GP32, GP16, or IFR followed by _<name>_ \n";
    print PRNT_F "   */ \n";
    print PRNT_F "  for (i = 0; ; i++) { \n";
    print PRNT_F "    c = name[i]; \n";
    print PRNT_F "    if (c == '_') break; \n";
    print PRNT_F "    buf[i] = c; \n";
    print PRNT_F "  } \n\n";

    print PRNT_F "  buf[i] = '\\0'; \n";
    print PRNT_F "  k = 0; \n";
    print PRNT_F "  // if this is an intrinsic, prepend __ to the name: \n";
    print PRNT_F "  if (!strcmp (buf, \"IFR\")) { \n";
    print PRNT_F "    buf[k++] = '_'; \n";
    print PRNT_F "    buf[k++] = '_'; \n";
    print PRNT_F "  } \n\n";

    print PRNT_F "  for (j = k; ; j++, i++) { \n";
    print PRNT_F "    c = name[i+1]; \n";
    print PRNT_F "    if (c == '\\0' || c == '_') break; \n";
    print PRNT_F "    buf[j] = c; \n";
    print PRNT_F "  } \n\n";

    print PRNT_F "  buf[j] = '\\0'; \n\n";

    print PRNT_F "  // convert to lower case \n";
    print PRNT_F "  for (i = 0; i <= j; i++) { \n";
    print PRNT_F "    lower_buf[i] = tolower(buf[i]); \n";
    print PRNT_F "  } \n\n";

    print PRNT_F "  return strdup(lower_buf); \n";
    print PRNT_F "} \n\n";

    print PRNT_F "main() \n";
    print PRNT_F "{ \n";

    print PRNT_F "  ISA_Print_Begin(\"st100\"); \n\n";

    print PRNT_F "  Set_AsmName_Func(asmname); \n\n";

    return;
}

# ==================================================================
#    finalize_print_file
# ==================================================================

sub finalize_print_file {

    print PRNT_F "\n\n";
    print PRNT_F "  ISA_Print_End(); \n";
    print PRNT_F "  return 0; \n";
    print PRNT_F "} \n";

    close (PRNT_F);
    return;
}

# ==================================================================
#    initialize_pack_file
# ==================================================================

sub initialize_pack_file {
    open (PACK_F, "> isa_pack.cxx");
    &copyright_notice (PACK_F);

    printf (PACK_F "// \n");
    printf (PACK_F "// Group TOPs with similar packing format together.  \n");
    printf (PACK_F "///////////////////////////////////////////////////////// \n");
    printf (PACK_F "// The instructions are listed by category. The different categories of \n");
    printf (PACK_F "// instructions are: \n");
    printf (PACK_F "// \n");
    printf (PACK_F "//   1. ALU \n");
    printf (PACK_F "//   2. Integer \n");
    printf (PACK_F "//   3. Memory \n");
    printf (PACK_F "//   4. Branch \n");
    printf (PACK_F "//   5. Float \n");
    printf (PACK_F "// \n");
    printf (PACK_F "// Within each Pack_Type instructions are listed in the order as shown \n");
    printf (PACK_F "// in the IA-64 instructions formats manual \n");
    printf (PACK_F "///////////////////////////////////// \n\n");


    printf (PACK_F "// Instructions may be packed (compressed) in binary files. The packing rules \n");
    printf (PACK_F "// are specified in this file. \n\n");

    printf (PACK_F "#include <stddef.h> \n");
    printf (PACK_F "#include \"topcode.h\" \n");
    printf (PACK_F "#include \"isa_pack_gen.h\" \n\n");
 
    printf (PACK_F "main() \n");
    printf (PACK_F "{ \n");
}

# ==================================================================
#    finalize_pack_file
# ==================================================================

sub finalize_pack_file {

    printf (PACK_F "  ISA_Pack_End(); \n");
    printf (PACK_F "  return 0; \n");
    printf (PACK_F "} \n");

    close (PACK_F);
    return;
}

# ==================================================================
#    initialize_bundle_file
# ==================================================================

sub initialize_bundle_file {
    open (BUNDLE_F, "> isa_bundle.cxx");
    &copyright_notice (BUNDLE_F);

    printf (BUNDLE_F "//  \n");
    printf (BUNDLE_F "//  Generate ISA bundle information \n");
    printf (BUNDLE_F "/////////////////////////////////////// \n\n");

    printf (BUNDLE_F "#include <stddef.h> \n");
    printf (BUNDLE_F "#include \"topcode.h\" \n");
    printf (BUNDLE_F "#include \"isa_bundle_gen.h\" \n\n");

    printf (BUNDLE_F "main() \n");
    printf (BUNDLE_F "{ \n");
}

# ==================================================================
#    finalize_bundle_file
# ==================================================================

sub finalize_bundle_file {

    printf (BUNDLE_F "  return 0; \n");
    printf (BUNDLE_F "} \n");

    close (BUNDLE_F);
    return;
}

# ==================================================================
#    initialize_decode_file
# ==================================================================

sub initialize_decode_file {
    open (DECODE_F, "> isa_decode.cxx");
    &copyright_notice (DECODE_F);

    printf (DECODE_F "// \n");
    printf (DECODE_F "// Generate instruction decoding information. \n");
    printf (DECODE_F "///////////////////////////////////// \n");
    printf (DECODE_F "///////////////////////////////////// \n\n");

    printf (DECODE_F "#include \"topcode.h\" \n");
    printf (DECODE_F "#include \"isa_decode_gen.h\" \n");
    printf (DECODE_F "#include \"targ_isa_bundle.h\" \n\n");
 
    printf (DECODE_F "main() \n");
    printf (DECODE_F "{ \n\n");
}

# ==================================================================
#    finalize_decode_file
# ==================================================================

sub finalize_decode_file {

    printf (DECODE_F "  return 0; \n");
    printf (DECODE_F "} \n");

    close (DECODE_F);
    return;
}

# ==================================================================
#    copyright_notice
# ==================================================================

sub copyright_notice {

    my $FILE = $_[0];

    printf($FILE "/* -- This file is automatically generated -- */ \n");
    printf($FILE "/* \n\n");

    printf($FILE "  Copyright (C) 2001 ST Microelectronics, Inc.  All Rights Reserved. \n\n");

    printf($FILE "  This program is free software; you can redistribute it and/or modify it \n");
    printf($FILE "  under the terms of version 2 of the GNU General Public License as \n");
    printf($FILE "  published by the Free Software Foundation. \n");

    printf($FILE "  This program is distributed in the hope that it would be useful, but \n");
    printf($FILE "  WITHOUT ANY WARRANTY; without even the implied warranty of \n");
    printf($FILE "  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. \n\n");

    printf($FILE "  Further, this software is distributed without any warranty that it is \n");
    printf($FILE "  free of the rightful claim of any third person regarding infringement \n");
    printf($FILE "  or the like.  Any license provided herein, whether implied or \n");
    printf($FILE "  otherwise, applies only to this software file.  Patent licenses, if \n");
    printf($FILE "  any, provided herein do not apply to combinations of this program with \n");
    printf($FILE "  other software, or any other product whatsoever. \n");

    printf($FILE "  You should have received a copy of the GNU General Public License along \n");
    printf($FILE "  with this program; if not, write the Free Software Foundation, Inc., 59 \n");
    printf($FILE "  Temple Place - Suite 330, Boston MA 02111-1307, USA. \n\n");

    printf($FILE "  Contact information:  ST Microelectronics, Inc., \n");
    printf($FILE "  , or: \n\n");

    printf($FILE "  http://www.st.com \n\n");

    printf($FILE "  For further information regarding this notice, see: \n\n");

    printf($FILE "  http: \n");

    printf($FILE "*/ \n\n");
}

# ==================================================================
#    DECL_ISA_SUBSET
#
#    An ISA subset is composed of a list of OPerations.
#    To each subset there are corresponding SUBSET_res and SUBSET_scd
#
#    array SUBSET_scd{$subset} = { int id; char *name; array res }
# ==================================================================

sub DECL_ISA_SUBSET {
    $SUBSET_name[$SUBSET_count] = $_[0];
    $SUBSET_count++;
}

# ==================================================================
#    subset_id
# ==================================================================

sub subset_id {
    my $name = $_[0];
    my $i;
    for ($i = 0; $i < $SUBSET_count; $i++) {
	if ($SUBSET_name[$i] eq $name) {
	    return $i;
	}
    }

    # couldn't find this subset:
    printf(STDOUT "ERROR: subset %s undeclared\n", $name);
    exit(1);
}

# ==================================================================
#    DECL_SCD_RESOURCE
# ==================================================================

sub DECL_SCD_RESOURCE {
    my $name = $_[0];                       # resource name
    my $avail = $_[1];                      # available units

    $RES_name[$RES_count] = $name;
    $RES_avail[$RES_count] = $avail;
    $RES_count++;
}

# ==================================================================
#    DECL_SCD_CLASS
# ==================================================================

sub DECL_SCD_CLASS {
    $SCD_CLASS_name[$SCD_CLASS_count] = $_[0];
    $SCD_CLASS_count++;
}

# ==================================================================
#    DECL_PACK_TYPE
# ==================================================================

sub DECL_PACK_TYPE {
    $PACK_type[$PACK_count] = $_[0];
    $PACK_comment[$PACK_count] = $_[1];
    $PACK_count++;
}

# ==================================================================
#    DECL_EXEC_SLOT
# ==================================================================

sub DECL_EXEC_SLOT {
    $EXEC_SLOT_name[$EXEC_SLOT_count] = $_[0];
    $EXEC_SLOT_scds[$EXEC_SLOT_count] = $_[1];
    $EXEC_SLOT_count++;
}

# ==================================================================
#    DECL_BUNDLE
# ==================================================================

sub DECL_BUNDLE {
    my $i;

    $BUNDLE_name[$BUNDLE_count] = $_[0];
    $BUNDLE_bits[$BUNDLE_count] = $_[1];
    $BUNDLE_field[$BUNDLE_count] = $_[2];
    $BUNDLE_slots[$BUNDLE_count] = $_[3];
    for ($i = 0; $i < $_[3]; $i++) {
	$BUNDLE_slot[$BUNDLE_count][$i] = $_[i+4];
    }
    $BUNDLE_temps[$BUNDLE_count] = 0;
    $cur_bundle = $BUNDLE_count;
    $BUNDLE_count++;
}

# ==================================================================
#    DECL_TEMPLATE
# ==================================================================

sub DECL_TEMPLATE {
    my $i;
    my $temp;

    $temp = $BUNDLE_temps[$cur_bundle];
    ${BUNDLE_temp[$cur_bundle][$temp]}{'name'} = $_[0];
    ${BUNDLE_temp[$cur_bundle][$temp]}{'slots'} = $_[1]; 

    # Initialize stops to no stops.
    # HACK for the ST100:
    # On the ST100 by default, all bundles have two variants: with
    # and without a stop at the end. The variant with the stop is
    # automatically instantiated during the emission.
    for ($i = 0; $i < $_[1]; $i++) {
	${BUNDLE_temp[$cur_bundle][$temp]}{'slot'}[$i]{'stop'} = 0;
    }
    $cur_temp = $BUNDLE_temps[$cur_bundle];
    $BUNDLE_temps[$cur_bundle]++;
}

# ==================================================================
#    SLOT
# ==================================================================

sub SLOT {
    my $i;

    # slot exceeds the number of allowed slots:
    if (${BUNDLE_temp[$cur_bundle][$cur_temp]}{'slots'} <= $_[0]) {
	printf(STDOUT "ERROR: invalid slot number %d\n", $_[0]);
	exit(1);
    }
    # slot refers to non-existing FU
    for ($i = 0; $i < $EXEC_SLOT_count; $i++) {
	if ($_[1] eq $EXEC_SLOT_name[$i]) {
	    goto BREAK;
	}
    }
    printf(STDOUT "ERROR: invalid slot kind %d\n", $_[1]);
    exit(1);

BREAK:
    ${BUNDLE_temp[$cur_bundle][$cur_temp]}{'slot'}[$_[0]]{'unit'} = $_[1];
}

# ==================================================================
#    STOP
# ==================================================================

sub STOP {
    # stop exceeds the number of allowed slots:
    if (${BUNDLE_temp[$cur_bundle][$cur_temp]}{'slots'} <= $_[0]) {
	printf(STDOUT "ERROR: invalid stop number %d\n", $_[0]);
	exit(1);
    }

    ${BUNDLE_temp[$cur_bundle][$cur_temp]}{'slot'}[$_[0]]{'stop'} = 1;
}

# ==================================================================
#    main
# ==================================================================

  if (!open (OPERATOR_F, "< ../src/Operator.ent")) {
      printf (STDOUT "ERROR: can't open file \"Operator.ent\" \n");
      exit(1);
  }

  ####################################################################
  #                         GLOBAL VARIABLES
  ####################################################################

  # global ISA subset count
  $SUBSET_count = 0;
  # global OPerations count
  $OP_count = 0;
  # global scheduling resource count
  $RES_count = 0;
  # global SCD_CLASS count
  $SCD_CLASS_count = 0;
  # global EXEC_SLOT count
  $EXEC_SLOT_count = 0;
  # global BUNDLE count -- can only be 1 for now
  $BUNDLE_count = 0;
  # global packing groups count (instruction binary compression)
  $PACK_count = 0;

  ####################################################################
  #  Operation properties -- 32 bits can map 32 properties:
  ####################################################################

  $OP_NONE      = 0x00000000;
#  $OP_INTRINSIC = 0x00000001;
  $OP_SIMULATED = 0x00000002;
  $OP_DUMMY     = 0x00000004;
  $OP_LOAD      = 0x00000010;
  $OP_STORE     = 0x00000020;

  ####################################################################
  #                        ISA DESCRIPTION
  #
  #     Declare the ISA subsets available on the machine.
  #     OPerations can belong to only one subset for now. 
  #     TODO: the compiler pseudo OPerations should be able to belong 
  #           to all ISA subsets ??. should they also be "dummy" ??
  #
  #     TODO: for each ISA subset we should have a separate files
  #           OPERATOR_F and SCHED_F. When declaring the
  #           ISA subset, indicate the file (can not do it now since
  #           gp32 and gp16 Operator file is one for both).
  ####################################################################

  &DECL_ISA_SUBSET ("gp32");
#  &DECL_ISA_SUBSET ("gp16");

  # read OPERATOR_F file:
  my $line;

  CONTINUE:
  while ($line = <OPERATOR_F>) {
      # Read one OPeration record:
      if (!($line =~ /<tr>/)) {
	  printf (STDOUT "ERROR: can not find record start \n");
	  exit(1);
      }

      # Read the record:
      my $line1 = <OPERATOR_F>;
      my $line2 = <OPERATOR_F>;
      my $line3 = <OPERATOR_F>;
      my $line4 = <OPERATOR_F>;
      my $line5 = <OPERATOR_F>;
      my $line6 = <OPERATOR_F>;
      my $line7 = <OPERATOR_F>;
      my $line8 = <OPERATOR_F>;
      my $line9 = <OPERATOR_F>;
      my $line10 = <OPERATOR_F>;
      my $line11 = <OPERATOR_F>;
      my $line12 = <OPERATOR_F>;
      my $line13 = <OPERATOR_F>;
      my $line14 = <OPERATOR_F>;
      my $line15 = <OPERATOR_F>;
      my $line16 = <OPERATOR_F>;
      my $line17 = <OPERATOR_F>;
      my $line18 = <OPERATOR_F>;
      my $line19 = <OPERATOR_F>;
      my $line20 = <OPERATOR_F>;
      my $line21 = <OPERATOR_F>;
      my $line22 = <OPERATOR_F>;

      $line = <OPERATOR_F>;
      if (!($line =~ /<\/tr>/)) {
	  printf (STDOUT "ERROR: corrupted record \n");
	  exit(1);
      }

      if ($line1 =~ /<td>(\w*)<\/td>/) {
	  $OP_opcode[$OP_count] = $1;
      }
      else {
	  print "  ERROR: can't find operator name !\n";
	  exit (1);
      }

      # I need to skip some operators:
      if (($OP_opcode[$OP_count] =~ /PSEUDO/) ||
	  ($OP_opcode[$OP_count] =~ /INFO/) ||
	  ($OP_opcode[$OP_count] =~ /DATA/) ||
          ($OP_opcode[$OP_count] =~ /IFR/) ||
          ($OP_opcode[$OP_count] =~ /GP16/)) {
	  next CONTINUE;
      }

      # reset OP_properties:
      $OP_properties[$OP_count] = OP_NONE;

      # read OPeration name:
      $line2 =~ /<td>\"(\w*)\"<\/td>/;
      $OP_name[$OP_count] = $1;

      # read OPeration assembly format:
      $line4 =~ /<td>\"(.*)\"<\/td>/;
      $OP_format[$OP_count] = $1;

      # read OPeration attributes:
      $line5 =~ /<td>(.*)<\/td>/;
      $OP_attr[$OP_count] = $1;

      # read OPeration operand count:
      $line6 =~ /<td>(\d*)<\/td>/;
      $OP_opcnt[$OP_count] = $1;

      # read operator predicate operand index
      $line7 =~ /<td>(\d+|-\d)<\/td>/;
      $OP_pred[$OP_count] = $1;

      # read operator target operand index
      $line8 =~ /<td>(\d+|-\d+)<\/td>/;
      $OP_target[$OP_count] = $1;

      # read OPeration base operand index
      $line11 =~ /<td>(\d+|-\d)<\/td>/;
      $OP_base[$OP_count] = $1;

      # read OPeration offset operand index
      $line12 =~ /<td>(\d+|-\d)<\/td>/;
      $OP_offset[$OP_count] = $1;

      # read OPeration memory access width
      $line13 =~ /<td>(\d*)<\/td>/;
      $OP_bytes[$OP_count] = $1;

      # read OPeration memory alignment
      $line14 =~ /<td>(\d*)<\/td>/;
      $OP_align[$OP_count] = $1;

      # read OPeration uses
      $line15 =~ /UsesDefs_(\w*)/;
      $OP_operands[$OP_count] = $1;

      # read OPeration defs
      $line16 =~ /UsesDefs_(\w*)/;
      $OP_results[$OP_count] = $1;

      # read OPeration scd class info
      $line17 =~ /ScdClass_(\w*)/;
      $OP_scdclass[$OP_count] = $1;

      $OP_count++;
  }

  ####################################################################
  #                       SCHEDULING MODEL
  #
  #  1. Declare resources available on the machine. This corresponds
  #     to the compiler scheduling abstraction. 
  #
  #  File SCHED_F describes scheduling classes for each machine
  #  mode (GP16/GP32/SLIW). Each Scd Class uses a subset of resources
  #  declared here. Function read_scdinfo() reads this information
  #  from SCHED_F file.
  #
  ####################################################################



  &DECL_SCD_RESOURCE ("ISSUE", 2);     # I0,I1
  &DECL_SCD_RESOURCE ("AU", 2);        # A0,A1
  &DECL_SCD_RESOURCE ("DU", 2);        # D0,D1
  &DECL_SCD_RESOURCE ("GU", 1);        # GU
  &DECL_SCD_RESOURCE ("G", 2);         # G0 or G1
  &DECL_SCD_RESOURCE ("B", 1);         # G0 and G1

#  &DECL_SCD_RESOURCE ("TI", 2);
#  &DECL_SCD_RESOURCE ("BRANCH", 1);
#  &DECL_SCD_RESOURCE ("SR", 1);
#  &DECL_SCD_RESOURCE ("SL", 1);
#  &DECL_SCD_RESOURCE ("EX", 1);
#  &DECL_SCD_RESOURCE ("MEMORY", 1);

  ###############################################################
  #                      EXECUTION MODEL
  #
  #  1. An instruction belongs to a unique SCD_CLASS. 
  #     DECL_SCD_CLASS () function declares an SCD_CLASS.
  #
  #  2. An SCD_CLASS may execute in one or more EXEC_SLOTs.
  #     DECL_EXEC_SLOT () function declares EXEC_SLOTs and
  #     SCD_CLASSes that can execute in this slot. 
  #     
  #  3. Specify bundle information ?? see isa_bundle.cxx
  #
  #  For now this is not an accurate execution model:
  #
  #     GP32: only this is implemented for now.
  #
  #     GP16, SLIW: not implemented.
  #
  ###############################################################

  &DECL_SCD_CLASS ("LCR");  # Load Control Register
  &DECL_SCD_CLASS ("LSR");  # Load Status Register
  &DECL_SCD_CLASS ("LBR");  # Load Boolean Register
  &DECL_SCD_CLASS ("LAR");  # Load Address Register
  &DECL_SCD_CLASS ("LDR");  # Load Data Register
  &DECL_SCD_CLASS ("SCR");  # Store Control Register
  &DECL_SCD_CLASS ("SSR");  # Store Status Register
  &DECL_SCD_CLASS ("SBR");  # Store Boolean Register
  &DECL_SCD_CLASS ("SAR");  # Store Address Register
  &DECL_SCD_CLASS ("SDR");  # Store Data Register
  &DECL_SCD_CLASS ("EXP");  # Expanded Instruction ??
  &DECL_SCD_CLASS ("D2A");  # copya, copysa
  &DECL_SCD_CLASS ("A2D");  # copyd
  &DECL_SCD_CLASS ("B2D");  # bool, boolp
  &DECL_SCD_CLASS ("AOP");  # AU Operation
  &DECL_SCD_CLASS ("AGM");  # AU GMI
  &DECL_SCD_CLASS ("SCE");  # Single Cycle Execute
  &DECL_SCD_CLASS ("SGM");  # Single Cycle GMI
  &DECL_SCD_CLASS ("DGM");  # Double Cycle GMI
  &DECL_SCD_CLASS ("DCE");  # Double Cycle Execute
  &DECL_SCD_CLASS ("MAC");  # Multiply Accumulate
  &DECL_SCD_CLASS ("FOP");  # Flag Operation
  &DECL_SCD_CLASS ("FGM");  # Flag Test GMI
  &DECL_SCD_CLASS ("BRA");  # Branch Operation
  &DECL_SCD_CLASS ("SUB");  # Subroutine Call
  &DECL_SCD_CLASS ("SWE");  # Software Exception
  &DECL_SCD_CLASS ("A2C");  # AU to Loop Counter
  &DECL_SCD_CLASS ("D2C");  # DU to Loop Counter
  &DECL_SCD_CLASS ("LCE");  # Set Loop Start/End
  &DECL_SCD_CLASS ("CIM");  # Change Instruction Mode
  &DECL_SCD_CLASS ("SMD");  # Change Instruction Mode to SLIW
  &DECL_SCD_CLASS ("PSR");  # PSR Register Update
  &DECL_SCD_CLASS ("SYN");  # Synchronize
  &DECL_SCD_CLASS ("NOP");  # No Operation
  &DECL_SCD_CLASS ("NULL");  
  
  # ScdClass slotting:
  &DECL_EXEC_SLOT ("S0", "LSR,SSR"); # SR in slot 0
  &DECL_EXEC_SLOT ("S1", "LSR");  # SR in slot 1
  &DECL_EXEC_SLOT ("E0", "LCR,LAR,SCR,AOP,SCE,DCE,MAC,FOP,A2C,D2C,LCE,NOP");
  &DECL_EXEC_SLOT ("E1", "LCR,LAR,SCR,SSR,SBR,SAR,SDR,AOP,SCE,DCE,MAC,FOP,A2C,D2C,LCE,NOP,EXP,BRA,SUB,SWE,CIM,SMD,SYN"); # only slot 1
  &DECL_EXEC_SLOT ("G", "B2D,SGM,DGM,FGM"); # G0 or G1
  &DECL_EXEC_SLOT ("B", "LBR"); # G0 and G1
  &DECL_EXEC_SLOT ("A0", "SBR,SAR,SDR"); # SL in slot 0
  &DECL_EXEC_SLOT ("A1", "LDR"); # SL in slot 1
  &DECL_EXEC_SLOT ("E0ND", "LDR"); # E0 + nodep
  &DECL_EXEC_SLOT ("D2A", "D2A"); # D2A_Unit
  &DECL_EXEC_SLOT ("A2D", "A2D"); # A2D_Unit
  &DECL_EXEC_SLOT ("T", "AGM"); # T_Unit

  # Declare bundle (name, size_in_bits, temp_bits, num_slots, [slots 0..N]).
  &DECL_BUNDLE("st100", 64, "0,0,0", 2, "0,0,32", "0,32,32");

  # Define possible templates (combinations of EXEC_SLOTs in BUNDLEs).

  # S0_Unit:
  &DECL_TEMPLATE("s0_e1",2);
    &SLOT(0, "S0");
    &SLOT(1, "E1");
    &STOP(1);
  &DECL_TEMPLATE("s0_a1",2);
    &SLOT(0, "S0");
    &SLOT(1, "A1");
    &STOP(1);
  &DECL_TEMPLATE("s0_g",2);
    &SLOT(0, "S0");
    &SLOT(1, "G");
    &STOP(1);
  &DECL_TEMPLATE("s0_b",2);
    &SLOT(0, "S0");
    &SLOT(1, "B");
    &STOP(1);
  &DECL_TEMPLATE("s0_d2a",2);
    &SLOT(0, "S0");
    &SLOT(1, "D2A");
    &STOP(1);
  &DECL_TEMPLATE("s0_a2d",2);
    &SLOT(0, "S0");
    &SLOT(1, "A2D");
    &STOP(1);
  &DECL_TEMPLATE("s0_t",2);
    &SLOT(0, "S0");
    &SLOT(1, "T");
    &STOP(1);

  # E0_Unit:
  &DECL_TEMPLATE("e0_s1",2);
    &SLOT(0, "E0");
    &SLOT(1, "S1");
    &STOP(1);
  &DECL_TEMPLATE("e0_e1",2);
    &SLOT(0, "E0");
    &SLOT(1, "E1");
    &STOP(1);
  &DECL_TEMPLATE("e0_a1",2);
    &SLOT(0, "E0");
    &SLOT(1, "A1");
    &STOP(1);
  &DECL_TEMPLATE("e0_g",2);
    &SLOT(0, "E0");
    &SLOT(1, "G");
    &STOP(1);
  &DECL_TEMPLATE("e0_b",2);
    &SLOT(0, "E0");
    &SLOT(1, "B");
    &STOP(1);
  &DECL_TEMPLATE("e0_d2a",2);
    &SLOT(0, "E0");
    &SLOT(1, "D2A");
    &STOP(1);
  &DECL_TEMPLATE("e0_a2d",2);
    &SLOT(0, "E0");
    &SLOT(1, "A2D");
    &STOP(1);
  &DECL_TEMPLATE("e0_t",2);
    &SLOT(0, "E0");
    &SLOT(1, "T");
    &STOP(1);

  # E0ND_Unit:
  &DECL_TEMPLATE("e0nd_s1",2);
    &SLOT(0, "E0ND");
    &SLOT(1, "S1");
    &STOP(1);
  &DECL_TEMPLATE("e0nd_e1",2);
    &SLOT(0, "E0ND");
    &SLOT(1, "E1");
    &STOP(1);
  &DECL_TEMPLATE("e0nd__e1",2);
    &SLOT(0, "E0ND");
    &STOP(0);                # LDR -> SDR
    &SLOT(1, "E1");
    &STOP(1);
  &DECL_TEMPLATE("e0nd_a1",2);
    &SLOT(0, "E0ND");
    &SLOT(1, "A1");
    &STOP(1);
  &DECL_TEMPLATE("e0nd_g",2);
    &SLOT(0, "E0ND");
    &SLOT(1, "G");
    &STOP(1);
  &DECL_TEMPLATE("e0nd_b",2);
    &SLOT(0, "E0ND");
    &SLOT(1, "B");
    &STOP(1);
  &DECL_TEMPLATE("e0nd_d2a",2);
    &SLOT(0, "E0ND");
    &SLOT(1, "D2A");
    &STOP(1);
  &DECL_TEMPLATE("e0nd__d2a",2);
    &SLOT(0, "E0ND");
    &STOP(0);                # LDR -> COPYA
    &SLOT(1, "D2A");
    &STOP(1);
  &DECL_TEMPLATE("e0nd_a2d",2);
    &SLOT(0, "E0ND");
    &SLOT(1, "A2D");
    &STOP(1);
  &DECL_TEMPLATE("e0nd_t",2);
    &SLOT(0, "E0ND");
    &SLOT(1, "T");
    &STOP(1);

  # A0_Unit:
  &DECL_TEMPLATE("a0_e1",2);
    &SLOT(0, "A0");
    &SLOT(1, "E1");
    &STOP(1);
  &DECL_TEMPLATE("a0_g",2);
    &SLOT(0, "A0");
    &SLOT(1, "G");
    &STOP(1);
  &DECL_TEMPLATE("a0_b",2);
    &SLOT(0, "A0");
    &SLOT(1, "B");
    &STOP(1);
  &DECL_TEMPLATE("a0_d2a",2);
    &SLOT(0, "A0");
    &SLOT(1, "D2A");
    &STOP(1);
  &DECL_TEMPLATE("a0_a2d",2);
    &SLOT(0, "A0");
    &SLOT(1, "A2D");
    &STOP(1);
  &DECL_TEMPLATE("a0_t",2);
    &SLOT(0, "A0");
    &SLOT(1, "T");
    &STOP(1);

  # G_Unit:
  &DECL_TEMPLATE("g_s1",2);
    &SLOT(0, "G");
    &SLOT(1, "S1");
    &STOP(1);
  &DECL_TEMPLATE("g_e1",2);
    &SLOT(0, "G");
    &SLOT(1, "E1");
    &STOP(1);
  &DECL_TEMPLATE("g_a1",2);
    &SLOT(0, "G");
    &SLOT(1, "A1");
    &STOP(1);
  &DECL_TEMPLATE("g_g",2);
    &SLOT(0, "G");
    &SLOT(1, "G");
    &STOP(1);
  &DECL_TEMPLATE("g_d2a",2);
    &SLOT(0, "G");
    &SLOT(1, "D2A");
    &STOP(1);
  &DECL_TEMPLATE("g_a2d",2);
    &SLOT(0, "G");
    &SLOT(1, "A2D");
    &STOP(1);
  &DECL_TEMPLATE("g_t",2);
    &SLOT(0, "G");
    &SLOT(1, "T");
    &STOP(1);

  # B_Unit:
  &DECL_TEMPLATE("b_s1",2);
    &SLOT(0, "B");
    &SLOT(1, "S1");
    &STOP(1);
  &DECL_TEMPLATE("b_e1",2);
    &SLOT(0, "B");
    &SLOT(1, "E1");
    &STOP(1);
  &DECL_TEMPLATE("b_a1",2);
    &SLOT(0, "B");
    &SLOT(1, "A1");
    &STOP(1);
  &DECL_TEMPLATE("b_d2a",2);
    &SLOT(0, "B");
    &SLOT(1, "D2A");
    &STOP(1);
  &DECL_TEMPLATE("b_a2d",2);
    &SLOT(0, "B");
    &SLOT(1, "A2D");
    &STOP(1);
  &DECL_TEMPLATE("b_t",2);
    &SLOT(0, "B");
    &SLOT(1, "T");
    &STOP(1);

  # D2A_Unit:
  &DECL_TEMPLATE("d2a_e1",2);
    &SLOT(0, "D2A");
    &SLOT(1, "E1");
    &STOP(1);
  &DECL_TEMPLATE("d2a_g",2);
    &SLOT(0, "D2A");
    &SLOT(1, "G");
    &STOP(1);
  &DECL_TEMPLATE("d2a_b",2);
    &SLOT(0, "D2A");
    &SLOT(1, "B");
    &STOP(1);
  &DECL_TEMPLATE("d2a_d2a",2);
    &SLOT(0, "D2A");
    &SLOT(1, "D2A");
    &STOP(1);

  # A2D_Unit:
  &DECL_TEMPLATE("a2d_s1",2);
    &SLOT(0, "A2D");
    &SLOT(1, "S1");
    &STOP(1);
  &DECL_TEMPLATE("a2d_e1",2);
    &SLOT(0, "A2D");
    &SLOT(1, "E1");
    &STOP(1);
  &DECL_TEMPLATE("a2d_a1",2);
    &SLOT(0, "A2D");
    &SLOT(1, "A1");
    &STOP(1);
  &DECL_TEMPLATE("a2d_g",2);
    &SLOT(0, "A2D");
    &SLOT(1, "G");
    &STOP(1);
  &DECL_TEMPLATE("a2d_b",2);
    &SLOT(0, "A2D");
    &SLOT(1, "B");
    &STOP(1);
  &DECL_TEMPLATE("a2d_a2d",2);
    &SLOT(0, "A2D");
    &SLOT(1, "A2D");
    &STOP(1);
  &DECL_TEMPLATE("a2d_t",2);
    &SLOT(0, "A2D");
    &SLOT(1, "T");
    &STOP(1);

  # T_Unit:
  &DECL_TEMPLATE("t_s1",2);
    &SLOT(0, "T");
    &SLOT(1, "S1");
    &STOP(1);
  &DECL_TEMPLATE("t_e1",2);
    &SLOT(0, "T");
    &SLOT(1, "E1");
    &STOP(1);
  &DECL_TEMPLATE("t_a1",2);
    &SLOT(0, "T");
    &SLOT(1, "A1");
    &STOP(1);
  &DECL_TEMPLATE("t_g",2);
    &SLOT(0, "T");
    &SLOT(1, "G");
    &STOP(1);
  &DECL_TEMPLATE("t_a2d",2);
    &SLOT(0, "T");
    &SLOT(1, "A2D");
    &STOP(1);
  &DECL_TEMPLATE("t_t",2);
    &SLOT(0, "T");
    &SLOT(1, "T");
    &STOP(1);

  # Read file that contains scheduling classes:
  &read_scdinfo ();

  ###############################################################
  #                ASSEMBLER/DESASSEMBLER MODEL
  #
  #  1. Specify the decoding transition rules ?? They are somehow
  #     related to execution unit types ?? see isa_decode.cxx
  #
  #  For now this is a completely dummy part.
  #
  ###############################################################

  &DECL_PACK_TYPE ("a1", "Integer ALU -- Register-Register");

  ###############################################################
  #                       PROCESSING
  ###############################################################

  # open and emit head info into all concerned files:
  &initialize_isa_file ();
  &initialize_si_file ();
  &initialize_operands_file ();
  &initialize_subset_file ();
  &initialize_properties_file ();
  &initialize_print_file ();
  &initialize_pack_file ();
  &initialize_bundle_file ();
  &initialize_decode_file ();

  # Sort ISA opcodes preparing them for emission
  for ($Opcode = 0; $Opcode < $OP_count; $Opcode++) {
      printf (STDOUT "<------ sorting %s ------> \n", $OP_opcode[$Opcode]);
      &sort_by_attributes ();
      &sort_by_operands ();
      &sort_by_scdclass ();
      &sort_by_format ();
      printf (STDOUT "<---------- done ---------> \n", $OP_opcode[$Opcode]);
  }

  # The Pro64 compiler needs a number of simulated and dummy 
  # opcodes. They get added after all the real ISA opcodes have
  # been sorted because simulated/dummy do not need to be.
  &init_required_opcodes ();


  # Emit ISA information
  for ($Opcode = 0; $Opcode < $OP_count; $Opcode++) {
      &emit_opcode ();
  }

# Opcodes that are required by the back end:
#&emit_required_opcodes ();

  # Emit the information for all opcodes including simulated/dummy:
  &emit_subsets ();
  &emit_properties ();
  &emit_operands ();

  # Emit the information for all opcodes not for simulated/dummy:
  &emit_scdinfo ();
  &emit_printing_formats ();
  &emit_pack_info ();
  &emit_bundle_info ();
  &emit_decode_info ();

  # end of generation:
  &finalize_isa_file ();
  &finalize_si_file ();
  &finalize_operands_file ();
  &finalize_subset_file ();
  &finalize_properties_file ();
  &finalize_print_file ();
  &finalize_pack_file ();
  &finalize_bundle_file ();
  &finalize_decode_file ();


