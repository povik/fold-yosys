/*
 *  yosys -- Yosys Open SYnthesis Suite
 *
 *  Copyright (C) 2012  Claire Xenia Wolf <claire@yosyshq.com>
 *
 *  Permission to use, copy, modify, and/or distribute this software for any
 *  purpose with or without fee is hereby granted, provided that the above
 *  copyright notice and this permission notice appear in all copies.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#include "kernel/yosys.h"

USING_YOSYS_NAMESPACE
PRIVATE_NAMESPACE_BEGIN

struct UniquifyPass : public Pass {
	UniquifyPass() : Pass("uniquify", "create unique copies of modules") { }
	void help() override
	{
		//   |---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|
		log("\n");
		log("    uniquify [selection]\n");
		log("\n");
		log("By default, a module that is instantiated by several other modules is only\n");
		log("kept once in the design. This preserves the original modularity of the design\n");
		log("and reduces the overall size of the design in memory. But it prevents certain\n");
		log("optimizations and other operations on the design. This pass creates unique\n");
		log("modules for all selected cells. The created modules are marked with the\n");
		log("'unique' attribute.\n");
		log("\n");
		log("This commands only operates on modules that by themself have the 'unique'\n");
		log("attribute set (the 'top' module is unique implicitly).\n");
		log("\n");
	}
	void execute(std::vector<std::string> args, RTLIL::Design *design) override
	{
		log_header(design, "Executing UNIQUIFY pass (creating unique copies of modules).\n");

		size_t argidx;
		for (argidx = 1; argidx < args.size(); argidx++)
		{
			// if (args[argidx] == "-check") {
			// 	flag_check = true;
			// 	continue;
			// }
			break;
		}
		extra_args(args, argidx, design);

		bool did_something = true;
		int count = 0;

		while (did_something)
		{
			did_something = false;

			for (auto module : design->selected_modules())
			{
				if (!module->get_bool_attribute(ID::unique) && !module->get_bool_attribute(ID::top))
					continue;

				for (auto cell : module->selected_cells())
				{
					Module *tmod = design->module(cell->type);
					IdString newname = module->name.str() + "." + log_id(cell->name);

					if (tmod == nullptr)
						continue;

					if (tmod->get_blackbox_attribute())
						continue;

					if (tmod->get_bool_attribute(ID::unique) && newname == tmod->name)
						continue;

					log("Creating module %s from %s.\n", log_id(newname), log_id(tmod));

					auto smod = tmod->clone();
					smod->name = newname;
					cell->type = newname;
					smod->set_bool_attribute(ID::unique);
					if (smod->attributes.count(ID::hdlname) == 0)
						smod->attributes[ID::hdlname] = string(log_id(tmod->name));
					design->add(smod);

					did_something = true;
					count++;
				}
			}
		}

		log("Created %d unique modules.\n", count);
	}
} UniquifyPass;

PRIVATE_NAMESPACE_END
