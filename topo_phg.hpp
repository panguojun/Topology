/**************************************************************************
*						【TOPO TREE的PHG脚本】 
* 
*					这个文件作为拓扑脚本来用！
*						{a{et:1,2},{b{et:5,6}c{et:7,8}},d{et:9,10}}
**************************************************************************/
struct tree_t;
namespace topo_phg
{
	struct transform_t { vec3 p; quaternion q; vec3 pyr; vec3 s = vec3::ONE; };
	// 资源
	struct res_t
	{
		// 携带的属性
		poly_t	poly;			// 多边形
		transform_t trans;		// 变换

		res_t() {}
		res_t(const res_t& v)
		{
			poly = v.poly;
			trans = v.trans;
		};
		~res_t() {}
	};
	vector<res_t*> reslist;		// 资源列表
	res_t& res(ENT& ent)
	{
		if (ent.resid == -1)
		{
			reslist.push_back(new res_t());
			ent.resid = reslist.size() - 1;
			//PRINT("create res id=" << resid);
		}
		ASSERT(ent.resid < reslist.size());
		return *reslist[ent.resid];
	}
	// 清理
	void clearres()
	{
		for (auto it : reslist)
			if (it) delete it;
		reslist.clear();
	}

	// *********************************************************************
	// Setup tree, generate entities
	// *********************************************************************
	// data convert
	int stoint(crstr sval)
	{
		return atoi(sval.c_str());
	}
	real storeal(crstr sval)
	{
		return atof(sval.c_str());
	}
	vec3 stovec(crstr sval)
	{
		vec3 ret;
		sscanf(sval.c_str(), "%f,%f,%f", &ret.x, &ret.y, &ret.z);
		return ret;
	}
	TOPO_TREE::edge_topo* GET_EDGE(TOPO_TREE::edge_topo* tree, crstr name)
	{
		if (tree->name == name)
			return tree;

		for (auto it : tree->children) {
			if (TOPO_TREE::edge_topo * e = GET_EDGE(it, name); e)
				return e;
		}
		return 0;
	}
	// ---------------------------------------------------------------------
	// 边拓扑
	// ---------------------------------------------------------------------
	void setupE(tree_t* tree, TOPO_TREE::edge_topo* parent_topo)
	{
		work_stack.push_back(tree);

		ENT ent;
		gvarmapstack.getvar(ent, tree->name.c_str());

		KV_IT;
		{
			// 边拓扑
			TOPO_TREE::edge_topo* topo = 0;
			KEY_VAL("vt") {
				TOPO_TREE::edge_topo::edge e;
				sscanf(it->second.c_str(),
				"%d,%d",
				&e.start, &e.end);
				topo = new TOPO_TREE::edge_topo({ tree->name, e });
				if (parent_topo){
					PRINT("edge_topo1: " << tree->name);
					parent_topo->children.push_back(topo);
					topo->parent = parent_topo;
				}
			} else {
				topo = new TOPO_TREE::edge_topo({ tree->name, {0} });
				if (parent_topo)
				{
					PRINT("edge_topo2: " << tree->name);
					parent_topo->children.push_back(topo);
					topo->parent = parent_topo;
				}
			}

			// 格点数量
			KEY_VAL("st") {
				if (it->second.find("*") != string::npos)
				{
					vector<string> slist;
					STR::split(it->second, slist, "*");

					int step = atoi(slist.front().c_str());
					int num = atoi(slist.back().c_str());
					//PRINTV(step)PRINTV(num);
					topo->space.steps = step * num - num + 1; // 格点数量 = L * N - N + 1，L为子分段格点数，N分段数 
					PRINTV(topo->space.steps)
				}
				else {
					sscanf(it->second.c_str(),
						"%d",
						&topo->space.steps);
				}
			}
			// 是否互斥
			KEY_VAL("mt") {
				topo->bmutex = true;
			}
			parent_topo = topo;

			//PRINT("addvar:" << tree->name);
			gvarmapstack.addvar(tree->name.c_str(), ent);
		}

		// children
		for (auto it : tree->children) {
			setupE(it.second, parent_topo);
		}
	}
	// ---------------------------------------------------------------------
	// setup
	// ---------------------------------------------------------------------
	void setup(tree_t* tree, const transform_t& parent, TOPO_TREE::surface_topo* surface, TOPO_TREE::edge_topo* parent_topo)
	{
		work_stack.push_back(tree);

		ENT ent;
		gvarmapstack.getvar(ent, tree->name.c_str());
		//getchar();
		KV_IT;
		transform_t& trans = res(ent).trans;
		{// transform
			vec3 p;
			quaternion q;
			vec3 pyr;
			vec3 s = vec3::ONE;
			{// transform desc
				KV_IT
					KEY_VAL("p") // raw position
				{
					p += stovec(it->second);
				}
				KEY_VAL("x") // move x
				{
					p += vec3::UX * storeal(it->second);
				}
				KEY_VAL("y") // move y
				{
					p += vec3::UY * storeal(it->second);
				}
				KEY_VAL("z") // move z
				{
					p += vec3::UZ * storeal(it->second);
				}
				KEY_VAL("pyr") // euler angles
				{
					pyr = stovec(it->second);
					q.fromeuler(pyr.x * PI / 180.0f, pyr.y * PI / 180.0f, pyr.z * PI / 180.0f);
				}
				KEY_VAL("pit") // pitch
				{
					quaternion _q;
					real ang = storeal(it->second) * PI / 180.0f;
					_q.ang_axis(ang, vec::UX);
					q = _q * q;
				}
				KEY_VAL("yaw") // yaw
				{
					quaternion _q;
					real ang = storeal(it->second) * PI / 180.0f;
					_q.ang_axis(ang, vec::UY);
					q = _q * q;
				}
				KEY_VAL("rol") // roll
				{
					quaternion _q;
					real ang = storeal(it->second) * PI / 180.0f;
					_q.ang_axis(ang, vec::UZ);
					q = _q * q;
				}
				KEY_VAL("s") // scale
				{
					s = vec3(storeal(it->second));
					PRINTVEC3(s);
				}
			}
			trans = {
				parent.p + (parent.q * vec3::UX) * p.x + (parent.q * vec3::UY) * p.y + (parent.q * vec3::UZ) * p.z,
				q * parent.q,
				parent.pyr + pyr, // 叠加？
				parent.s * s
			};
		}
		{// 添加到变量列表
			KEY_VAL("tri2") {
				triangle_t t;
				sscanf(it->second.c_str(),
					"(%f,%f),(%f,%f),(%f,%f)",
					&t.p[0].p.x, &t.p[0].p.y,
					&t.p[1].p.x, &t.p[1].p.y,
					&t.p[2].p.x, &t.p[2].p.y);
				color = RNDCOR;
				t.getploy(res(ent).poly);

			}
			KEY_VAL("tri") {
				triangle_t t;
				sscanf(it->second.c_str(),
					"(%f,%f,%f),(%f,%f,%f),(%f,%f,%f)",
					&t.p[0].p.x, &t.p[0].p.y, &t.p[0].p.z,
					&t.p[1].p.x, &t.p[1].p.y, &t.p[1].p.z,
					&t.p[2].p.x, &t.p[2].p.y, &t.p[2].p.z);
				color = RNDCOR;
				t.getploy(res(ent).poly);
				estack.push_back(res(ent).poly.e);
			}
			KEY_VAL("poly") {
				std::vector<std::string> out;
				STR::split(it->second, out, "\n");
				for (auto& it : out)
				{
					PRINTV(it)
						vec3 p;
					sscanf(it.c_str(),
						"%f,%f,%f",
						&p.x, &p.y, &p.z);
					res(ent).poly.e.push_back(p);
				}
				PRINTV(res(ent).poly.e.size());
				estack.push_back(res(ent).poly.e);
			}
			KEY_VAL("round") {
				VECLIST e;
				real r;
				int sig;
				sscanf(it->second.c_str(),
					"%f,%d",
					&r, &sig);
				roundedge(e, vec::ZERO, vec::UX, vec::UY, r, sig, 0);
				res(ent).poly.e = e;
			}
		}
		{
			// -------------------------------------------
			// 顶点集合
			// -------------------------------------------	
			KEY_VAL("V")
			{
				GTable::p_set.clear();
				std::vector<std::string> out;
				STR::split(it->second, out, " ");
				for (auto& it : out)
				{
					vec3 p = ScePHG::constraint_algebra::stovec(it);
					GTable::p_set.push_back(p);
				}
			}
			// -------------------------------------------
			// 边拓扑
			// -------------------------------------------
			TOPO_TREE::edge_topo tedge({ tree->name, {0} });
			setupE(tree, &tedge);
			{
				constraint cst;
				cst.cd = coord3(trans.p, trans.q);
				GTable::cst_list.push_back(cst);
			}
			// -------------------------------------------
			// 面列表
			// -------------------------------------------	
			KEY_VAL("F")
			{
				std::vector<std::string> slist;
				STR::split(it->second, slist, " ");
				for (auto& it : slist)
				{
					TOPO_TREE::surface_topo* surface = new TOPO_TREE::surface_topo();
					std::vector<std::string> epair;
					STR::split(it, epair, ",");
					
					PRINT(epair[0] << "," << epair[1]);

					surface->tlist.push_back(GET_EDGE(&tedge, epair[0]));
					surface->tlist.push_back(GET_EDGE(&tedge, epair[1]));

					TOPO_TREE::topo_shapes.resize(TOPO_TREE::topo_shapes.size() + 1);
					TOPO_TREE::topo_shape& tshape = TOPO_TREE::topo_shapes.back();
					tshape.topo_surfs.push_back(surface);
				}
			}
			TOPO_TREE::edge_list.push_back(std::move(tedge));
			//PRINT("addvar:" << tree->name);
			gvarmapstack.addvar(tree->name.c_str(), ent);
		}
	}
}
