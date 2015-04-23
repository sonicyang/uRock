FATOUTDIR = FATLib

FATSRCDIR = Middlewares/Third_Party/FatFs/src \
			Middlewares/Third_Party/FatFs/src/drivers \
			Middlewares/Third_Party/FatFs/src/option

FATINCDIR = Middlewares/Third_Party/FatFs/src \
			Middlewares/Third_Party/FatFs/src/drivers \
			Middlewares/Third_Party/FatFs/src/option

FATSRC += $(wildcard $(addsuffix /*.c,$(FATSRCDIR))) \
		  $(wildcard $(addsuffix /*.s,$(FATSRCDIR)))

INCLUDES += $(addprefix -I,$(FATINCDIR))

FATOBJS += $(addprefix $(OUTDIR)/$(FATOUTDIR)/,$(patsubst %.s,%.o,$(FATSRC:.c=.o)))

$(OUTDIR)/$(FATOUTDIR)/%.o: %.c
	@echo "   FAT   |   CC    "$@	
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -MMD -MF $@.d -c $(INCLUDES) $< -o $@

.PHONY: clean-fat
clean-fat:
	@rm -rf $(OUTDIR)/$(FATOUTDIR)
	@echo "Removing FAT Library Object Files"
