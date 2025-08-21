import * as React from 'react';
import type { JoinedGame } from './types';

interface PlayerListProps {
  players: JoinedGame['players'];
  creatorConnectionId: string;
  peerStatuses:
    | ('connecting' | 'connected' | 'disconnected' | undefined)[]
    | undefined;
  currentConnectionId: string | undefined;
}

export const PlayerList: React.FC<PlayerListProps> = ({
  players,
  creatorConnectionId,
  peerStatuses,
  currentConnectionId,
}) => {
  return (
    <div>
      <h3 className="text-lg font-semibold mb-4">Players</h3>
      <div className="space-y-3">
        {players.map((player, index) => {
          if (!player) {
            return (
              <div
                key={`empty-${index}`}
                className="flex items-center justify-between p-3 border rounded-lg opacity-50"
              >
                <div className="flex items-center space-x-3">
                  <div className="w-8 h-8 bg-primary/10 rounded-full flex items-center justify-center">
                    <span className="text-sm font-medium">{index + 1}</span>
                  </div>
                  <div>
                    <p className="font-medium text-muted-foreground">
                      Empty Slot
                    </p>
                  </div>
                </div>
                <div className="flex items-center space-x-2">
                  <div className="w-2 h-2 bg-gray-400 rounded-full"></div>
                  <span className="text-sm text-muted-foreground">Empty</span>
                </div>
              </div>
            );
          }

          return (
            <div
              key={player.connectionId}
              className="flex items-center justify-between p-3 border rounded-lg"
            >
              <div className="flex items-center space-x-3">
                <div className="w-8 h-8 bg-primary/10 rounded-full flex items-center justify-center">
                  <span className="text-sm font-medium">{index + 1}</span>
                </div>
                <div>
                  <p className="font-medium">
                    Player {index + 1}
                    {player.connectionId === creatorConnectionId && (
                      <span className="ml-2 px-2 py-1 text-xs bg-primary/20 rounded">
                        Host
                      </span>
                    )}
                  </p>
                  <p className="text-sm text-muted-foreground">
                    ID: {player.connectionId}
                  </p>
                </div>
              </div>
              <div className="flex items-center space-x-2">
                {(() => {
                  if (player.connectionId === currentConnectionId) {
                    return (
                      <>
                        <div className="w-2 h-2 bg-blue-500 rounded-full"></div>
                        <span className="text-sm text-muted-foreground">
                          You
                        </span>
                      </>
                    );
                  }

                  const status = peerStatuses?.[index] || 'disconnected';
                  const statusColor =
                    status === 'connected'
                      ? 'bg-green-500'
                      : status === 'connecting'
                        ? 'bg-yellow-500'
                        : 'bg-red-500';
                  const statusText =
                    status === 'connected'
                      ? 'Connected'
                      : status === 'connecting'
                        ? 'Connecting'
                        : 'Disconnected';
                  return (
                    <>
                      <div
                        className={`w-2 h-2 ${statusColor} rounded-full`}
                      ></div>
                      <span className="text-sm text-muted-foreground">
                        {statusText}
                      </span>
                    </>
                  );
                })()}
              </div>
            </div>
          );
        })}
      </div>
    </div>
  );
};