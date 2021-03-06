/*
	Copyright 2009-2012, Sumeet Chhetri

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
/*
 * CORSHandler.cpp
 *
 *  Created on: 16-Apr-2013
 *      Author: sumeetc
 */

#include "CORSHandler.h"

CORSHandler::CORSHandler() {

}

CORSHandler::~CORSHandler() {
}

bool CORSHandler::handle(HttpRequest *req, HttpResponse *res, ConfigurationData configData) {
	int reqType = req->getCORSRequestType();
	if(reqType == PREFLIGHT)
	{
		strVec reqOrgLst = req->parseHeaderValue(req->getHeader(HttpRequest::Origin));
		if(configData.corsConfig.isOriginAllowed(reqOrgLst))
		{
			if(req->getHeader(HttpResponse::AccessControlAllowMethods)=="")
			{
				HTTPResponseStatus status(HTTPResponseStatus::BadRequest, "Invalid preflight CORS request: Missing Access-Control-Request-Method header");
				throw status;
			}
			else
			{
				if(!HttpRequest::isValidHttpMethod(req->getHeader(HttpResponse::AccessControlAllowMethods)))
				{
					HTTPResponseStatus status(HTTPResponseStatus::InvalidMethod, "Unsupported HTTP method: " + req->getHeader(HttpResponse::AccessControlAllowMethods));
					throw status;
				}

				strVec reqHdrLst = req->parseHeaderValue(req->getHeader(HttpResponse::AccessControlAllowHeaders));

				if(!configData.corsConfig.isMethodAllowed(req->getHeader(HttpResponse::AccessControlAllowMethods)))
				{
					HTTPResponseStatus status(HTTPResponseStatus::InvalidMethod, "Unsupported HTTP method: " + req->getHeader(HttpResponse::AccessControlAllowMethods));
					throw status;
				}

				string erheadr;
				if(!configData.corsConfig.isHeaderAllowed(reqHdrLst, erheadr))
				{
					HTTPResponseStatus status(HTTPResponseStatus::Forbidden, "Unsupported HTTP request header: " + erheadr);
					throw status;
				}

				if(configData.corsConfig.allwdCredentials)
				{
					res->addHeaderValue(HttpResponse::AccessControlAllowCredentials, "true");
				}
				if(configData.corsConfig.allwdOrigins=="*")
				{
					res->addHeaderValue(HttpResponse::AccessControlAllowOrigin, "*");
				}
				else
				{
					res->addHeaderValue(HttpResponse::AccessControlAllowOrigin, req->getHeader(HttpRequest::Origin));
				}

				if(configData.corsConfig.maxAge>0)
				{
					res->addHeaderValue(HttpResponse::AccessControlAllowMethods, CastUtil::lexical_cast<string>(configData.corsConfig.maxAge));
				}

				res->addHeaderValue(HttpResponse::AccessControlAllowMethods, configData.corsConfig.allwdMethods);
				res->addHeaderValue(HttpResponse::AccessControlMaxAge, configData.corsConfig.allwdHeaders);

			}
		}
		else
		{
			HTTPResponseStatus status(HTTPResponseStatus::Forbidden, "CORS origin denied");
			throw status;
		}

		return true;
	}
	else if(reqType == CORS)
	{
		strVec reqOrgLst = req->parseHeaderValue(req->getHeader(HttpRequest::Origin));
		if(configData.corsConfig.isOriginAllowed(reqOrgLst))
		{
			if(!configData.corsConfig.isMethodAllowed(req->getMethod()))
			{
				HTTPResponseStatus status(HTTPResponseStatus::InvalidMethod, "Unsupported HTTP method: " + req->getMethod());
				throw status;
			}

			if(configData.corsConfig.allwdCredentials)
			{
				res->addHeaderValue(HttpResponse::AccessControlAllowCredentials, "true");
			}

			res->addHeaderValue(HttpResponse::AccessControlAllowOrigin, req->getHeader(HttpRequest::Origin));
			res->addHeaderValue(HttpResponse::AccessControlAllowHeaders, configData.corsConfig.exposedHeaders);
		}
		else
		{
			HTTPResponseStatus status(HTTPResponseStatus::Forbidden, "CORS origin denied");
			throw status;
		}
	}
	return false;
}
