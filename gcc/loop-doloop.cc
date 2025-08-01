/* Perform doloop optimizations
   Copyright (C) 2004-2023 Free Software Foundation, Inc.
   Based on code by Michael P. Hayes (m.hayes@elec.canterbury.ac.nz)

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 3, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING3.  If not see
<http://www.gnu.org/licenses/>.  */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "backend.h"
#include "target.h"
#include "rtl.h"
#include "tree.h"
#include "cfghooks.h"
#include "memmodel.h"
#include "emit-rtl.h"
#include "dojump.h"
#include "expr.h"
#include "cfgloop.h"
#include "cfgrtl.h"
#include "dumpfile.h"
#include "loop-unroll.h"
#include "regs.h"
#include "df.h"
#include "targhooks.h"

/* This module is used to modify loops with a determinable number of
   iterations to use special low-overhead looping instructions.

   It first validates whether the loop is well behaved and has a
   determinable number of iterations (either at compile or run-time).
   It then modifies the loop to use a low-overhead looping pattern as
   follows:

   1. A pseudo register is allocated as the loop iteration counter.

   2. The number of loop iterations is calculated and is stored
      in the loop counter.

   3. At the end of the loop, the jump insn is replaced by the
      doloop_end pattern.  The compare must remain because it might be
      used elsewhere.  If the loop-variable or condition register are
      used elsewhere, they will be eliminated by flow.

   4. An optional doloop_begin pattern is inserted at the top of the
      loop.

   TODO The optimization should only performed when either the biv used for exit
   condition is unused at all except for the exit test, or if we do not have to
   change its value, since otherwise we have to add a new induction variable,
   which usually will not pay up (unless the cost of the doloop pattern is
   somehow extremely lower than the cost of compare & jump, or unless the bct
   register cannot be used for anything else but doloop -- ??? detect these
   cases).  */

/* Return the loop termination condition for PATTERN or zero
   if it is not a decrement and branch jump insn.  */

rtx
doloop_condition_get (rtx_insn *doloop_pat)
{
  rtx cmp;
  rtx inc;
  rtx reg;
  rtx inc_src;
  rtx condition;
  rtx pattern;
  rtx cc_reg = NULL_RTX;
  rtx reg_orig = NULL_RTX;

  /* The canonical doloop pattern we expect has one of the following
     forms:

     1)  (parallel [(set (pc) (if_then_else (condition)
	  			            (label_ref (label))
				            (pc)))
	             (set (reg) (plus (reg) (const_int -1)))
	             (additional clobbers and uses)])

     The branch must be the first entry of the parallel (also required
     by jump.cc), and the second entry of the parallel must be a set of
     the loop counter register.  Some targets (IA-64) wrap the set of
     the loop counter in an if_then_else too.

     2)  (set (reg) (plus (reg) (const_int -1))
         (set (pc) (if_then_else (reg != 0)
	                         (label_ref (label))
			         (pc))).  

     Some targets (ARM) do the comparison before the branch, as in the
     following form:

     3) (parallel [(set (cc) (compare ((plus (reg) (const_int -1), 0)))
                   (set (reg) (plus (reg) (const_int -1)))])
        (set (pc) (if_then_else (cc == NE)
                                (label_ref (label))
                                (pc))) */

  pattern = PATTERN (doloop_pat);

  if (GET_CODE (pattern) != PARALLEL)
    {
      rtx cond;
      rtx_insn *prev_insn = prev_nondebug_insn (doloop_pat);
      rtx cmp_arg1, cmp_arg2;
      rtx cmp_orig;

      /* In case the pattern is not PARALLEL we expect two forms
	 of doloop which are cases 2) and 3) above: in case 2) the
	 decrement immediately precedes the branch, while in case 3)
	 the compare and decrement instructions immediately precede
	 the branch.  */

      if (prev_insn == NULL_RTX || !INSN_P (prev_insn))
        return 0;

      cmp = pattern;
      if (GET_CODE (PATTERN (prev_insn)) == PARALLEL)
        {
	  /* The third case: the compare and decrement instructions
	     immediately precede the branch.  */
	  cmp_orig = XVECEXP (PATTERN (prev_insn), 0, 0);
	  if (GET_CODE (cmp_orig) != SET)
	    return 0;
	  if (GET_CODE (SET_SRC (cmp_orig)) != COMPARE)
	    return 0;
	  cmp_arg1 = XEXP (SET_SRC (cmp_orig), 0);
          cmp_arg2 = XEXP (SET_SRC (cmp_orig), 1);
	  if (cmp_arg2 != const0_rtx 
	      || GET_CODE (cmp_arg1) != PLUS)
	    return 0;
	  reg_orig = XEXP (cmp_arg1, 0);
	  if (XEXP (cmp_arg1, 1) != GEN_INT (-1) 
	      || !REG_P (reg_orig))
	    return 0;
	  cc_reg = SET_DEST (cmp_orig);
	  
	  inc = XVECEXP (PATTERN (prev_insn), 0, 1);
	}
      else
        inc = PATTERN (prev_insn);
      if (GET_CODE (cmp) == SET && GET_CODE (SET_SRC (cmp)) == IF_THEN_ELSE)
	{
	  /* We expect the condition to be of the form (reg != 0)  */
	  cond = XEXP (SET_SRC (cmp), 0);
	  if (GET_CODE (cond) != NE || XEXP (cond, 1) != const0_rtx)
	    return 0;
	}
    }
  else
    {
      cmp = XVECEXP (pattern, 0, 0);
      inc = XVECEXP (pattern, 0, 1);
    }

  /* Check for (set (reg) (something)).  */
  if (GET_CODE (inc) != SET)
    return 0;
  reg = SET_DEST (inc);
  if (! REG_P (reg))
    return 0;

  /* Check if something = (plus (reg) (const_int -1)).
     On IA-64, this decrement is wrapped in an if_then_else.  */
  inc_src = SET_SRC (inc);
  if (GET_CODE (inc_src) == IF_THEN_ELSE)
    inc_src = XEXP (inc_src, 1);
  if (GET_CODE (inc_src) != PLUS
      || XEXP (inc_src, 0) != reg
      || XEXP (inc_src, 1) != constm1_rtx)
    return 0;

  /* Check for (set (pc) (if_then_else (condition)
                                       (label_ref (label))
                                       (pc))).  */
  if (GET_CODE (cmp) != SET
      || SET_DEST (cmp) != pc_rtx
      || GET_CODE (SET_SRC (cmp)) != IF_THEN_ELSE
      || GET_CODE (XEXP (SET_SRC (cmp), 1)) != LABEL_REF
      || XEXP (SET_SRC (cmp), 2) != pc_rtx)
    return 0;

  /* Extract loop termination condition.  */
  condition = XEXP (SET_SRC (cmp), 0);

  /* We expect a GE or NE comparison with 0 or 1.  */
  if ((GET_CODE (condition) != GE
       && GET_CODE (condition) != NE)
      || (XEXP (condition, 1) != const0_rtx
          && XEXP (condition, 1) != const1_rtx))
    return 0;

  if ((XEXP (condition, 0) == reg)
      /* For the third case:  */  
      || ((cc_reg != NULL_RTX)
	  && (XEXP (condition, 0) == cc_reg)
	  && (reg_orig == reg))
      || (GET_CODE (XEXP (condition, 0)) == PLUS
	  && XEXP (XEXP (condition, 0), 0) == reg))
   {
     if (GET_CODE (pattern) != PARALLEL)
     /*  For the second form we expect:

         (set (reg) (plus (reg) (const_int -1))
         (set (pc) (if_then_else (reg != 0)
                                 (label_ref (label))
                                 (pc))).

         is equivalent to the following:

         (parallel [(set (pc) (if_then_else (reg != 1)
                                            (label_ref (label))
                                            (pc)))
                     (set (reg) (plus (reg) (const_int -1)))
                     (additional clobbers and uses)])

        For the third form we expect:

        (parallel [(set (cc) (compare ((plus (reg) (const_int -1)), 0))
                   (set (reg) (plus (reg) (const_int -1)))])
        (set (pc) (if_then_else (cc == NE)
                                (label_ref (label))
                                (pc))) 

        which is equivalent to the following:

        (parallel [(set (cc) (compare (reg,  1))
                   (set (reg) (plus (reg) (const_int -1)))
                   (set (pc) (if_then_else (NE == cc)
                                           (label_ref (label))
                                           (pc))))])

        So we return the second form instead for the two cases.

     */
        condition = gen_rtx_fmt_ee (NE, VOIDmode, inc_src, const1_rtx);

    return condition;
   }

  /* ??? If a machine uses a funny comparison, we could return a
     canonicalized form here.  */

  return 0;
}

/* Return nonzero if the loop specified by LOOP is suitable for
   the use of special low-overhead looping instructions.  DESC
   describes the number of iterations of the loop.  */

static bool
doloop_valid_p (class loop *loop, class niter_desc *desc)
{
  basic_block *body = get_loop_body (loop), bb;
  rtx_insn *insn;
  unsigned i;
  bool result = true;

  /* Check for loops that may not terminate under special conditions.  */
  if (!desc->simple_p
      || desc->assumptions
      || desc->infinite)
    {
      /* There are some cases that would require a special attention.
	 For example if the comparison is LEU and the comparison value
	 is UINT_MAX then the loop will not terminate.  Similarly, if the
	 comparison code is GEU and the comparison value is 0, the
	 loop will not terminate.

	 If the absolute increment is not 1, the loop can be infinite
	 even with LTU/GTU, e.g. for (i = 3; i > 0; i -= 2)

	 ??? We could compute these conditions at run-time and have a
	 additional jump around the loop to ensure an infinite loop.
	 However, it is very unlikely that this is the intended
	 behavior of the loop and checking for these rare boundary
	 conditions would pessimize all other code.

	 If the loop is executed only a few times an extra check to
	 restart the loop could use up most of the benefits of using a
	 count register loop.  Note however, that normally, this
	 restart branch would never execute, so it could be predicted
	 well by the CPU.  We should generate the pessimistic code by
	 default, and have an option, e.g. -funsafe-loops that would
	 enable count-register loops in this case.  */
      if (dump_file)
	fprintf (dump_file, "Doloop: Possible infinite iteration case.\n");
      result = false;
      goto cleanup;
    }

  for (i = 0; i < loop->num_nodes; i++)
    {
      bb = body[i];

      for (insn = BB_HEAD (bb);
	   insn != NEXT_INSN (BB_END (bb));
	   insn = NEXT_INSN (insn))
	{
	  /* Different targets have different necessities for low-overhead
	     looping.  Call the back end for each instruction within the loop
	     to let it decide whether the insn prohibits a low-overhead loop.
	     It will then return the cause for it to emit to the dump file.  */
	  const char * invalid = targetm.invalid_within_doloop (insn);
	  if (invalid)
	    {
	      if (dump_file)
		fprintf (dump_file, "Doloop: %s\n", invalid);
	      result = false;
	      goto cleanup;
	    }
	}
    }
  result = true;

cleanup:
  free (body);

  return result;
}

/* Adds test of COND jumping to DEST on edge *E and set *E to the new fallthru
   edge.  If the condition is always false, do not do anything.  If it is always
   true, redirect E to DEST and return false.  In all other cases, true is
   returned.  */

static bool
add_test (rtx cond, edge *e, basic_block dest)
{
  rtx_insn *seq, *jump;
  rtx_code_label *label;
  machine_mode mode;
  rtx op0 = XEXP (cond, 0), op1 = XEXP (cond, 1);
  enum rtx_code code = GET_CODE (cond);
  basic_block bb;
  /* The jump is supposed to handle an unlikely special case.  */
  profile_probability prob = profile_probability::guessed_never ();

  mode = GET_MODE (XEXP (cond, 0));
  if (mode == VOIDmode)
    mode = GET_MODE (XEXP (cond, 1));

  start_sequence ();
  op0 = force_operand (op0, NULL_RTX);
  op1 = force_operand (op1, NULL_RTX);
  label = block_label (dest);
  do_compare_rtx_and_jump (op0, op1, code, 0, mode, NULL_RTX, NULL, label,
			   prob);

  jump = get_last_insn ();
  if (!jump || !JUMP_P (jump))
    {
      /* The condition is always false and the jump was optimized out.  */
      end_sequence ();
      return true;
    }

  seq = get_insns ();
  unshare_all_rtl_in_chain (seq);
  end_sequence ();

  /* There always is at least the jump insn in the sequence.  */
  gcc_assert (seq != NULL_RTX);

  bb = split_edge_and_insert (*e, seq);
  *e = single_succ_edge (bb);

  if (any_uncondjump_p (jump) && onlyjump_p (jump))
    {
      /* The condition is always true.  */
      delete_insn (jump);
      redirect_edge_and_branch_force (*e, dest);
      return false;
    }

  JUMP_LABEL (jump) = label;

  LABEL_NUSES (label)++;

  edge e2 = make_edge (bb, dest, (*e)->flags & ~EDGE_FALLTHRU);
  e2->probability = prob;
  (*e)->probability = prob.invert ();
  update_br_prob_note (e2->src);
  return true;
}

/* Fold (add -1; zero_ext; add +1) operations to zero_ext if not wrapping. i.e:

   73: r145:SI=r123:DI#0-0x1
   74: r144:DI=zero_extend (r145:SI)
   75: r143:DI=r144:DI+0x1
   ...
   31: r135:CC=cmp (r123:DI,0)
   72: {pc={(r143:DI!=0x1)?L70:pc};r143:DI=r143:DI-0x1;...}

   r123:DI#0-0x1 is param count derived from loop->niter_expr equal to number of
   loop iterations, if loop iterations expression doesn't overflow, then
   (zero_extend (r123:DI#0-1))+1 can be simplified to zero_extend.  */

static rtx
doloop_simplify_count (class loop *loop, scalar_int_mode mode, rtx count)
{
  widest_int iterations;
  if (GET_CODE (count) == ZERO_EXTEND)
    {
      rtx extop0 = XEXP (count, 0);
      if (GET_CODE (extop0) == PLUS)
	{
	  rtx addop0 = XEXP (extop0, 0);
	  rtx addop1 = XEXP (extop0, 1);

	  if (get_max_loop_iterations (loop, &iterations)
	      && wi::ltu_p (iterations, GET_MODE_MASK (GET_MODE (addop0)))
	      && addop1 == constm1_rtx)
	    return simplify_gen_unary (ZERO_EXTEND, mode, addop0,
				       GET_MODE (addop0));
	}
    }

  return simplify_gen_binary (PLUS, mode, count, const1_rtx);
}

/* Modify the loop to use the low-overhead looping insn where LOOP
   describes the loop, DESC describes the number of iterations of the
   loop, and DOLOOP_INSN is the low-overhead looping insn to emit at the
   end of the loop.  CONDITION is the condition separated from the
   DOLOOP_SEQ.  COUNT is the number of iterations of the LOOP.  */

static void
doloop_modify (class loop *loop, class niter_desc *desc,
	       rtx_insn *doloop_seq, rtx condition, rtx count)
{
  rtx counter_reg;
  rtx tmp, noloop = NULL_RTX;
  rtx_insn *sequence;
  rtx_insn *jump_insn;
  rtx_code_label *jump_label;
  int nonneg = 0;
  bool increment_count;
  basic_block loop_end = desc->out_edge->src;
  scalar_int_mode mode;
  widest_int iterations;

  jump_insn = BB_END (loop_end);

  if (dump_file)
    {
      fprintf (dump_file, "Doloop: Inserting doloop pattern (");
      if (desc->const_iter)
	fprintf (dump_file, "%" PRId64, desc->niter);
      else
	fputs ("runtime", dump_file);
      fputs (" iterations).\n", dump_file);
    }

  /* Discard original jump to continue loop.  The original compare
     result may still be live, so it cannot be discarded explicitly.  */
  delete_insn (jump_insn);

  counter_reg = XEXP (condition, 0);
  if (GET_CODE (counter_reg) == PLUS)
    counter_reg = XEXP (counter_reg, 0);
  /* These patterns must operate on integer counters.  */
  mode = as_a <scalar_int_mode> (GET_MODE (counter_reg));

  increment_count = false;
  switch (GET_CODE (condition))
    {
    case NE:
      /* Currently only NE tests against zero and one are supported.  */
      noloop = XEXP (condition, 1);
      if (noloop != const0_rtx)
	{
	  gcc_assert (noloop == const1_rtx);
	  increment_count = true;
	}
      break;

    case GE:
      /* Currently only GE tests against zero are supported.  */
      gcc_assert (XEXP (condition, 1) == const0_rtx);

      noloop = constm1_rtx;

      /* The iteration count does not need incrementing for a GE test.  */
      increment_count = false;

      /* Determine if the iteration counter will be non-negative.
	 Note that the maximum value loaded is iterations_max - 1.  */
      if (get_max_loop_iterations (loop, &iterations)
	  && wi::leu_p (iterations,
			wi::set_bit_in_zero <widest_int>
			(GET_MODE_PRECISION (mode) - 1)))
	nonneg = 1;
      break;

      /* Abort if an invalid doloop pattern has been generated.  */
    default:
      gcc_unreachable ();
    }

  if (increment_count)
    count = doloop_simplify_count (loop, mode, count);

  /* Insert initialization of the count register into the loop header.  */
  start_sequence ();
  /* count has been already copied through copy_rtx.  */
  reset_used_flags (count);
  set_used_flags (condition);
  tmp = force_operand (count, counter_reg);
  convert_move (counter_reg, tmp, 1);
  sequence = get_insns ();
  unshare_all_rtl_in_chain (sequence);
  end_sequence ();
  emit_insn_after (sequence, BB_END (loop_preheader_edge (loop)->src));

  if (desc->noloop_assumptions)
    {
      rtx ass = copy_rtx (desc->noloop_assumptions);
      basic_block preheader = loop_preheader_edge (loop)->src;
      basic_block set_zero = split_edge (loop_preheader_edge (loop));
      basic_block new_preheader = split_edge (loop_preheader_edge (loop));
      edge te;

      /* Expand the condition testing the assumptions and if it does not pass,
	 reset the count register to 0.  */
      redirect_edge_and_branch_force (single_succ_edge (preheader), new_preheader);
      set_immediate_dominator (CDI_DOMINATORS, new_preheader, preheader);

      set_zero->count = profile_count::uninitialized ();

      te = single_succ_edge (preheader);
      for (; ass; ass = XEXP (ass, 1))
	if (!add_test (XEXP (ass, 0), &te, set_zero))
	  break;

      if (ass)
	{
	  /* We reached a condition that is always true.  This is very hard to
	     reproduce (such a loop does not roll, and thus it would most
	     likely get optimized out by some of the preceding optimizations).
	     In fact, I do not have any testcase for it.  However, it would
	     also be very hard to show that it is impossible, so we must
	     handle this case.  */
	  set_zero->count = preheader->count;
	}

      if (EDGE_COUNT (set_zero->preds) == 0)
	{
	  /* All the conditions were simplified to false, remove the
	     unreachable set_zero block.  */
	  delete_basic_block (set_zero);
	}
      else
	{
	  /* Reset the counter to zero in the set_zero block.  */
	  start_sequence ();
	  convert_move (counter_reg, noloop, 0);
	  sequence = get_insns ();
	  end_sequence ();
	  emit_insn_after (sequence, BB_END (set_zero));

	  set_immediate_dominator (CDI_DOMINATORS, set_zero,
				   recompute_dominator (CDI_DOMINATORS,
							set_zero));
	}

      set_immediate_dominator (CDI_DOMINATORS, new_preheader,
			       recompute_dominator (CDI_DOMINATORS,
						    new_preheader));
    }

  /* Some targets (eg, C4x) need to initialize special looping
     registers.  */
  if (targetm.have_doloop_begin ())
    if (rtx_insn *seq = targetm.gen_doloop_begin (counter_reg, doloop_seq))
      emit_insn_after (seq, BB_END (loop_preheader_edge (loop)->src));

  /* Insert the new low-overhead looping insn.  */
  emit_jump_insn_after (doloop_seq, BB_END (loop_end));
  jump_insn = BB_END (loop_end);
  jump_label = block_label (desc->in_edge->dest);
  JUMP_LABEL (jump_insn) = jump_label;
  LABEL_NUSES (jump_label)++;

  /* Ensure the right fallthru edge is marked, for case we have reversed
     the condition.  */
  desc->in_edge->flags &= ~EDGE_FALLTHRU;
  desc->out_edge->flags |= EDGE_FALLTHRU;

  /* Add a REG_NONNEG note if the actual or estimated maximum number
     of iterations is non-negative.  */
  if (nonneg)
    add_reg_note (jump_insn, REG_NONNEG, NULL_RTX);

  /* Update the REG_BR_PROB note.  */
  if (desc->in_edge->probability.initialized_p ())
    add_reg_br_prob_note (jump_insn, desc->in_edge->probability);
}

/* Called through note_stores.  */

static void
record_reg_sets (rtx x, const_rtx pat ATTRIBUTE_UNUSED, void *data)
{
  bitmap mod = (bitmap)data;
  if (REG_P (x))
    {
      unsigned int regno = REGNO (x);
      if (HARD_REGISTER_P (x))
	{
	  unsigned int end_regno = end_hard_regno (GET_MODE (x), regno);
	  do
	    bitmap_set_bit (mod, regno);
	  while (++regno < end_regno);
	}
      else
	bitmap_set_bit (mod, regno);
    }
}

/* Process loop described by LOOP validating that the loop is suitable for
   conversion to use a low overhead looping instruction, replacing the jump
   insn where suitable.  Returns true if the loop was successfully
   modified.  */

static bool
doloop_optimize (class loop *loop)
{
  scalar_int_mode mode;
  rtx doloop_reg;
  rtx count;
  widest_int iterations, iterations_max;
  rtx_code_label *start_label;
  rtx condition;
  unsigned level;
  HOST_WIDE_INT est_niter;
  int max_cost;
  class niter_desc *desc;
  unsigned word_mode_size;
  unsigned HOST_WIDE_INT word_mode_max;
  int entered_at_top;

  if (dump_file)
    fprintf (dump_file, "Doloop: Processing loop %d.\n", loop->num);

  iv_analysis_loop_init (loop);

  /* Find the simple exit of a LOOP.  */
  desc = get_simple_loop_desc (loop);

  /* Check that loop is a candidate for a low-overhead looping insn.  */
  if (!doloop_valid_p (loop, desc))
    {
      if (dump_file)
	fprintf (dump_file,
		 "Doloop: The loop is not suitable.\n");
      return false;
    }
  mode = desc->mode;

  est_niter = get_estimated_loop_iterations_int (loop);
  if (est_niter == -1)
    est_niter = get_likely_max_loop_iterations_int (loop);

  if (est_niter >= 0 && est_niter < 3)
    {
      if (dump_file)
	fprintf (dump_file,
		 "Doloop: Too few iterations (%u) to be profitable.\n",
		 (unsigned int)est_niter);
      return false;
    }

  max_cost
    = COSTS_N_INSNS (param_max_iterations_computation_cost);
  if (set_src_cost (desc->niter_expr, mode, optimize_loop_for_speed_p (loop))
      > max_cost)
    {
      if (dump_file)
	fprintf (dump_file,
		 "Doloop: number of iterations too costly to compute.\n");
      return false;
    }

  if (desc->const_iter)
    iterations = widest_int::from (rtx_mode_t (desc->niter_expr, mode),
				   UNSIGNED);
  else
    iterations = 0;
  if (!get_max_loop_iterations (loop, &iterations_max))
    iterations_max = 0;
  level = get_loop_level (loop) + 1;
  entered_at_top = (loop->latch == desc->in_edge->dest
		    && contains_no_active_insn_p (loop->latch));
  if (!targetm.can_use_doloop_p (iterations, iterations_max, level,
				 entered_at_top))
    {
      if (dump_file)
	fprintf (dump_file, "Loop rejected by can_use_doloop_p.\n");
      return false;
    }

  /* Generate looping insn.  If the pattern FAILs then give up trying
     to modify the loop since there is some aspect the back-end does
     not like.  */
  count = copy_rtx (desc->niter_expr);
  start_label = block_label (desc->in_edge->dest);
  doloop_reg = gen_reg_rtx (mode);
  rtx_insn *doloop_seq = targetm.gen_doloop_end (doloop_reg, start_label);

  word_mode_size = GET_MODE_PRECISION (word_mode);
  word_mode_max = (HOST_WIDE_INT_1U << (word_mode_size - 1) << 1) - 1;
  if (! doloop_seq
      && mode != word_mode
      /* Before trying mode different from the one in that # of iterations is
	 computed, we must be sure that the number of iterations fits into
	 the new mode.  */
      && (word_mode_size >= GET_MODE_PRECISION (mode)
 	  || wi::leu_p (iterations_max, word_mode_max)))
    {
      if (word_mode_size > GET_MODE_PRECISION (mode))
	count = simplify_gen_unary (ZERO_EXTEND, word_mode, count, mode);
      else
	count = lowpart_subreg (word_mode, count, mode);
      PUT_MODE (doloop_reg, word_mode);
      doloop_seq = targetm.gen_doloop_end (doloop_reg, start_label);
    }
  if (! doloop_seq)
    {
      if (dump_file)
	fprintf (dump_file,
		 "Doloop: Target unwilling to use doloop pattern!\n");
      return false;
    }

  /* If multiple instructions were created, the last must be the
     jump instruction.  */
  rtx_insn *doloop_insn = doloop_seq;
  while (NEXT_INSN (doloop_insn) != NULL_RTX)
    doloop_insn = NEXT_INSN (doloop_insn);
  if (!JUMP_P (doloop_insn)
      || !(condition = doloop_condition_get (doloop_insn)))
    {
      if (dump_file)
	fprintf (dump_file, "Doloop: Unrecognizable doloop pattern!\n");
      return false;
    }

  /* Ensure that the new sequence doesn't clobber a register that
     is live at the end of the block.  */
  {
    bitmap modified = BITMAP_ALLOC (NULL);

    for (rtx_insn *i = doloop_seq; i != NULL; i = NEXT_INSN (i))
      note_stores (i, record_reg_sets, modified);

    basic_block loop_end = desc->out_edge->src;
    bool fail = bitmap_intersect_p (df_get_live_out (loop_end), modified);
    /* iv_analysis_loop_init calls df_analyze_loop, which computes just
       partial df for blocks of the loop only.  The above will catch if
       any of the modified registers are use inside of the loop body, but
       it will most likely not have accurate info on registers used
       at the destination of the out_edge.  We call df_analyze on the
       whole function at the start of the pass though and iterate only
       on innermost loops or from innermost loops, so
       live in on desc->out_edge->dest should be still unmodified from
       the initial df_analyze.  */
    if (!fail)
      fail = bitmap_intersect_p (df_get_live_in (desc->out_edge->dest),
				 modified);
    BITMAP_FREE (modified);

    if (fail)
      {
	if (dump_file)
	  fprintf (dump_file, "Doloop: doloop pattern clobbers live out\n");
	return false;
      }
  }

  doloop_modify (loop, desc, doloop_seq, condition, count);
  return true;
}

/* This is the main entry point.  Process all loops using doloop_optimize.  */

void
doloop_optimize_loops (void)
{
  if (optimize == 1)
    {
      df_live_add_problem ();
      df_live_set_all_dirty ();
    }

  df_analyze ();

  for (auto loop : loops_list (cfun,
			       targetm.can_use_doloop_p
			       == can_use_doloop_if_innermost
			       ? LI_ONLY_INNERMOST : LI_FROM_INNERMOST))
    doloop_optimize (loop);

  if (optimize == 1)
    df_remove_problem (df_live);

  iv_analysis_done ();

  checking_verify_loop_structure ();
}
