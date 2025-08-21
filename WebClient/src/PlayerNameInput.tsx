import * as React from 'react';
import { Button } from '@/components/ui/button';
import { Input } from '@/components/ui/input';
import { Label } from '@/components/ui/label';
import { useState, useEffect } from 'react';

interface PlayerNameInputProps {
  currentPlayerName: string;
  onUpdatePlayer: (name: string) => Promise<void>;
  disabled?: boolean;
}

export const PlayerNameInput: React.FC<PlayerNameInputProps> = ({
  currentPlayerName,
  onUpdatePlayer,
  disabled = false,
}) => {
  // Player name state and localStorage persistence
  const [playerName, setPlayerName] = useState(() => {
    return localStorage.getItem('hoverrace-player-name') || '';
  });
  const [isUpdatingName, setIsUpdatingName] = useState(false);

  // Update localStorage when player name changes
  useEffect(() => {
    if (playerName) {
      localStorage.setItem('hoverrace-player-name', playerName);
    }
  }, [playerName]);

  // Update server when name changes and is different from current
  const handleUpdateName = async () => {
    if (
      !playerName.trim() ||
      playerName === currentPlayerName ||
      isUpdatingName ||
      disabled
    ) {
      return;
    }

    setIsUpdatingName(true);
    try {
      await onUpdatePlayer(playerName.trim());
    } catch (error) {
      console.error('Failed to update player name:', error);
    } finally {
      setIsUpdatingName(false);
    }
  };

  return (
    <div className="space-y-2">
      <Label htmlFor="player-name">Your Name</Label>
      <div className="flex gap-2">
        <Input
          id="player-name"
          type="text"
          placeholder="Enter your name..."
          value={playerName}
          onChange={(e) => setPlayerName(e.target.value)}
          onBlur={handleUpdateName}
          onKeyDown={(e) => {
            if (e.key === 'Enter') {
              handleUpdateName();
              e.currentTarget.blur();
            }
          }}
          disabled={isUpdatingName || disabled}
        />
        <Button
          onClick={handleUpdateName}
          disabled={
            !playerName.trim() ||
            playerName === currentPlayerName ||
            isUpdatingName ||
            disabled
          }
          variant="outline"
          size="sm"
        >
          {isUpdatingName ? 'Updating...' : 'Update'}
        </Button>
      </div>
    </div>
  );
};
