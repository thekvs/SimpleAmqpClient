
/*
 * ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1
 *
 * ``The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is SimpleAmqpClient for RabbitMQ.
 *
 * The Initial Developer of the Original Code is Alan Antonuk.
 * Original code is Copyright (C) Alan Antonuk.
 *
 * All Rights Reserved.
 *
 * Contributor(s): ______________________________________.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU General Public License Version 2 or later (the "GPL"), in
 * which case the provisions of the GPL are applicable instead of those
 * above. If you wish to allow use of your version of this file only
 * under the terms of the GPL, and not to allow others to use your
 * version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the
 * notice and other provisions required by the GPL. If you do not
 * delete the provisions above, a recipient may use your version of
 * this file under the terms of any one of the MPL or the GPL.
 *
 * ***** END LICENSE BLOCK *****
 */

#include "SimpleAmqpClient/SimpleRpcServer.h"


namespace AmqpClient {

SimpleRpcServer::SimpleRpcServer(Channel::ptr_t channel, const std::string& rpc_name) :
	m_channel(channel)
  , m_incoming_tag(m_channel->DeclareQueue(rpc_name))

{
	m_channel->BindQueue(m_incoming_tag, "amq.direct", m_incoming_tag);
	m_channel->BasicConsume(m_incoming_tag, m_incoming_tag);
}

SimpleRpcServer::~SimpleRpcServer()
{
}

BasicMessage::ptr_t SimpleRpcServer::GetNextIncomingMessage()
{
	return m_channel->BasicConsumeMessage();
}

bool SimpleRpcServer::GetNextIncomingMessage(BasicMessage::ptr_t& message, int timeout)
{
	return m_channel->BasicConsumeMessage(message, timeout);
}

void SimpleRpcServer::RespondToMessage(BasicMessage::ptr_t request, BasicMessage::ptr_t response)
{
	if (request->CorrelationIdIsSet() && !response->CorrelationIdIsSet())
	{
		response->CorrelationId(request->CorrelationId());
	}

	m_channel->BasicPublish("amq.direct", request->ReplyTo(), response);
}

void SimpleRpcServer::RespondToMessage(BasicMessage::ptr_t request, const std::string response)
{
	BasicMessage::ptr_t response_message = BasicMessage::Create();
	response_message->Body(response);

	RespondToMessage(request, response_message);
}

} // namespace AmqpClient
