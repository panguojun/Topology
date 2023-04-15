/**
*						【几何体】
* 
*				几何体由拓扑，点集，约束三者生成
*/
namespace geomertry
{
	struct shape
	{
		TOPO_TREE::topo_shape* tshape;		// 拓扑
		vec3* pset;				// 点集
		constraint* cstlist;	// 约束

		void generate_face(TOPO_TREE::surface_topo* surf, submesh& sm)
		{
			PRINT("SSSSSSSSSSSSSSSSSSSS	Generate Surface " << surf->tlist.size() << " SSSSSSSSSSSSSSSSSSSS");
			//getchar();
			VECLIST e0;
			TOPO_TREE::edge_topo* et0 = 0;
			for (auto et : surf->tlist)
			{
				PRINT("CREATE EDGE(L) : " << et->name)
					VECLIST e;
				// 沿着左侧遍历
				et->walkdownL(et, [this, &e, &sm, et](crstr name, int start, int end) // 遍历拓扑结构
					{
						PRINT("---------Lwalk: " << name << " " << start << "," << end);
						et->space.walk([&e, et, start, end](real u)					// 遍历空间结构
							{
								crvec p1 = GTable::p_set[start - 1];
								crvec p2 = GTable::p_set[end - 1];
								coord3 cd = GTable::cst_list[et->cs].getCD(u);				// 计算点的坐标变换
								vec3 p = blend(p1, p2, u) * cd;
								PRINT("e1: (" << e.size() + 1 << ")" << p.x << "," << p.y << "," << p.z);
								if (e.empty() || e.back().p != p)
									e.push_back(p);
							});
					});
				if (et0)
				{
					if (et0->is_compound())
					{
						PRINT("CREATE EDGE(R) : " << et0->name)
							e0.clear();
						// 沿着右侧遍历
						et0->walkdownR(et0, [this, &e0, &sm, et0](crstr name, int start, int end) // 遍历拓扑结构
							{
								PRINT("----------Rwalk: " << name << " " << start << "," << end);
								et0->space.walk([&e0, et0, start, end](real u)			// 遍历空间结构
									{
										crvec p1 = GTable::p_set[start - 1];
										crvec p2 = GTable::p_set[end - 1];
										coord3 cd = GTable::cst_list[et0->cs].getCD(u);		// 计算点的坐标变换
										vec3 p = blend(p1, p2, u) * cd;
										PRINT("e0: (" << e0.size() + 1 << ")" << p.x << "," << p.y << "," << p.z);
										if (e0.empty() || e0.back().p != p)
											e0.push_back(p);
									}, -1);
							});
						invedge(e0);
					}

					PRINT("================== face: " << e0.size() << " : " << e.size());
					// 绘制代码
					//SETSM(sm);
					face(e0, e);
					//for (int i = 0; i < e0.size(); i ++)
					//{
					//	PRINT("e0: " << e0[i].p.x << "," << e0[i].p.y << "," << e0[i].p.z);
					//	//pt3d(e0[i].p, 0.1);
					//}
					//for (int i = 0; i < e.size(); i++)
					//{
					//	PRINT("e: " << e[i].p.x << "," << e[i].p.y << "," << e[i].p.z);
					//	//pt3d(e[i].p, 0.1);
					//}
					//PRINTV(SUBMESH.vertices.size());
					//RESM;
				}
				e0 = e;
				et0 = et;
			}
		}
		void generate(submesh& sm)
		{
			for (auto& v : GTable::p_set)
			{
				pt3d(v, 0.2);
			}

			for (auto surfit : tshape->topo_surfs)
				generate_face(surfit, sm);

			tshape->topo_surfs.clear();
		}
	};

	// ------------------------------
	// test
	// ------------------------------
	void test_generate()
	{
		shape shp;
		shp.tshape = &TOPO_TREE::topo_shapes[0];
		shp.generate(SUBMESH);

		for (auto& e : TOPO_TREE::edge_list)
			e.clear();
		TOPO_TREE::edge_list.clear();
	}
}
