#include "sgct/ReadConfig.h"
#include <tinyxml.h>

ReadConfig::ReadConfig( const std::string filename )
{
	valid = false;

	if( filename.empty() )
	{
		fprintf(stderr, "Error: No XML config file loaded.\n");
		return;
	}
	
	xmlFileName = filename;

	try
	{
		readAndParseXML();
		valid = true;
		fprintf(stderr, "Config file '%s' read successfully!\n", xmlFileName.c_str());
		fprintf(stderr, "Number of nodes in cluster: %d\n", nodes.size());
		for(unsigned int i = 0; i<nodes.size(); i++)
			fprintf(stderr, "Node(%d) ip: %s\n", i, nodes[i].ip.c_str());
	}
	catch(char * err)
	{
		fprintf(stderr, "Error occured while reading config file '%s'\nError: %s\n", xmlFileName.c_str(), err);
	}
}

void ReadConfig::readAndParseXML()
{
	TiXmlDocument initVals( xmlFileName.c_str() );
	if( !initVals.LoadFile() )
	{
		throw initVals.ErrorDesc();
		return;
	}

	TiXmlElement* XMLroot = initVals.FirstChildElement( "Cluster" );
	if( XMLroot == NULL )
	{
		throw "Cannot find XML root!";
		return;
	}

	masterIP.assign( XMLroot->Attribute( "masterAddress" ) );
	masterPort.assign( XMLroot->Attribute( "port" ) );

	TiXmlElement* element[3];
	const char * val[3];
	element[0] = XMLroot->FirstChildElement();
	while( element[0] != NULL )
	{
		val[0] = element[0]->Value();

		if( strcmp("Node", val[0]) == 0 )
		{
			NodeConfig tmpNodeCfg;
			tmpNodeCfg.master = (strcmp( element[0]->Attribute( "type" ), "Master" ) == 0 ? true : false); 
			tmpNodeCfg.ip.assign( element[0]->Attribute( "ip" ) );

			element[1] = element[0]->FirstChildElement();
			while( element[1] != NULL )
			{
				val[1] = element[1]->Value();

				if( strcmp("Window", val[1]) == 0 )
				{
					tmpNodeCfg.fullscreen = (strcmp( element[1]->Attribute("fullscreen"), "true" ) == 0 ? true : false);
					element[1]->Attribute("numberOfSamples", &tmpNodeCfg.numberOfSamples );
					tmpNodeCfg.useSwapGroups = (strcmp( element[1]->Attribute("swapLock"), "true" ) == 0 ? true : false);
					
					element[2] = element[1]->FirstChildElement();
					while( element[2] != NULL )
					{
						val[2] = element[2]->Value();

						if( strcmp("Stereo", val[2]) == 0 )
						{
							tmpNodeCfg.stereo = getStereoType( element[2]->Attribute("type") );
						}
						else if( strcmp("Size", val[2]) == 0 )
						{
							element[2]->Attribute("x", &tmpNodeCfg.windowData[2] );
							element[2]->Attribute("y", &tmpNodeCfg.windowData[3] );
						}
						else if( strcmp("Pos", val[2]) == 0 )
						{
							element[2]->Attribute("x", &tmpNodeCfg.windowData[0] );
							element[2]->Attribute("y", &tmpNodeCfg.windowData[1] );
						}

						//iterate
						element[2] = element[2]->NextSiblingElement();
					}
				}
				else if(strcmp("Viewplane", val[1]) == 0)
				{
					element[2] = element[1]->FirstChildElement();
					while( element[2] != NULL )
					{
						if( strcmp("Pos", val[2]) == 0 )
						{
							Point3f tmpP3;
							double dTmp[3];
							static unsigned int i=0;
							element[2]->Attribute("x", &dTmp[0]);
							element[2]->Attribute("y", &dTmp[1]);
							element[2]->Attribute("z", &dTmp[2]);

							tmpP3.x = static_cast<float>(dTmp[0]);
							tmpP3.y = static_cast<float>(dTmp[1]);
							tmpP3.z = static_cast<float>(dTmp[2]);

							tmpNodeCfg.viewPlaneCoords[i%3] = tmpP3;
							i++;
						}

						//iterate
						element[2] = element[2]->NextSiblingElement();
					}
				}
				
				//iterate
				element[1] = element[1]->NextSiblingElement();
			}

			nodes.push_back( tmpNodeCfg );
		}//end if node
		else if( strcmp("User", val[0]) == 0 )
		{
			double dTmp;
			element[0]->Attribute("eyeSeparation", &dTmp);
			eyeSeparation = static_cast<float>( dTmp );
			
			element[1] = element[0]->FirstChildElement();
			while( element[1] != NULL )
			{
				val[1] = element[1]->Value();
				
				if( strcmp("Pos", val[1]) == 0 )
				{
					double dTmp[3];
					element[1]->Attribute("x", &dTmp[0]);
					element[1]->Attribute("y", &dTmp[1]);
					element[1]->Attribute("z", &dTmp[2]);

					userPos.x = static_cast<float>(dTmp[0]);
					userPos.y = static_cast<float>(dTmp[1]);
					userPos.z = static_cast<float>(dTmp[2]);
				}

				//iterate
				element[1] = element[1]->NextSiblingElement();
			}
		}

		//iterate
		element[0] = element[0]->NextSiblingElement();
	}

	//fprintf(stderr, "Done\n" );
}

int ReadConfig::getStereoType( const std::string type )
{
	if( strcmp( type.c_str(), "none" ) == 0 )
		return None;
	else if( strcmp( type.c_str(), "active" ) == 0 )
		return Active;
	else if( strcmp( type.c_str(), "passive_vertical" ) == 0 )
		return PassiveVertical;
	else if( strcmp( type.c_str(), "passive_horizontal" ) == 0 )
		return PassiveHorizontal;
	else if( strcmp( type.c_str(), "checkerboard" ) == 0 )
		return Checkerboard;
	
	//if match not found
	return -1;
}