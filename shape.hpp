/**
*						【几何体】
* 
*				几何体由拓扑，点集，约束三者生成
*/
namespace geomertry
{
	struct shape
	{
		topoG* tshape;								// 拓扑
		vec3* p_set = &GTable::p_set[0];			// 点集
		constraint* cstlist = &GTable::cst_list[0];	// 约束

		void generate_face(topoF* surf, submesh& sm)
		{
			PRINT("SSSSSSSSSSSSSSSSSSSS	Generate Surface " << surf->tedges.size() << " SSSSSSSSSSSSSSSSSSSS");
			//getchar();
			VECLIST e0;
			topoE* et0 = 0;
			for (auto et : surf->tedges)
			{
				PRINT("CREATE EDGE(L) : " << et->name);
				VECLIST e;
				// 沿着左侧遍历
				et->walkL(et, [this, &e, &sm, et](topoE* to, int start, int end) // 遍历拓扑结构
					{
						PRINT("---------Lwalk: " << to->name << " " << start << "," << end);
						to->space.walk([this, &e, to, start, end](int i, real u)					// 遍历空间结构
							{
								crvec p1 = this->p_set[start - 1];
								crvec p2 = this->p_set[end - 1];
								coord3 cd = cstlist[to->cs].getCD(u);				// 计算点的坐标变换
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
						PRINT("CREATE EDGE(R) : " << et0->name);
						e0.clear();
						// 沿着右侧遍历
						et0->walkR(et0, [this, &e0, &sm, et0](topoE* to, int start, int end) // 遍历拓扑结构
							{
								PRINT("----------Rwalk: " << to->name << " " << start << "," << end);
								to->space.walk([this, &e0, to, start, end](int i, real u)			// 遍历空间结构
									{
										crvec p1 = p_set[start - 1];
										crvec p2 = p_set[end - 1];
										coord3 cd = cstlist[to->cs].getCD(u);			// 计算点的坐标变换
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

			for (auto surfit : tshape->tsurfaces)
				generate_face(surfit, sm);

			tshape->tsurfaces.clear();
		}
	};

	// ------------------------------
	// test
	// ------------------------------
	void test_generate()
	{
		shape shp;
		shp.tshape = &TOPO_TREE::tshapes[0];
		shp.generate(SUBMESH);

		for (auto& e : TOPO_TREE::tedges)
			e.clear();
		TOPO_TREE::tedges.clear();
	}
}